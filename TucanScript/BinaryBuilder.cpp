#include "BinaryBuilder.h"

using namespace TucanScript;

#define VERIFICATION_HEADER Sym { 0x25u }

Undef TucanScript::Binary::BinaryBuilder::Decompose (const String& inFilePath, VM::Asm& asm_, VM::ReadOnlyData& roData) {
    IFileStream inFileStream (inFilePath, std::ios::binary);

    if (!inFileStream) {
        LogErr ("Error opening file: " << inFilePath);
        return;
    }

    inFileStream.seekg (Zero, std::ios::end);
    const SInt64 bufferSize = inFileStream.tellg ();
    if (bufferSize <= Zero) {
        LogErr ("Invalid buffer size!");
        return;
    }
    inFileStream.seekg (Zero, std::ios::beg);

    Sym* buffer = new Sym[bufferSize];
    inFileStream.read (buffer, bufferSize);
    inFileStream.close ();

    //Literals processing
    QWord qCaret = 1;
    Sym verificationToken = buffer[Zero];
    if (verificationToken != VERIFICATION_HEADER) {
        LogErr ("Invalid verification token!");
        return;
    }
    Size nLiterals {};
    std::memcpy (&nLiterals, &buffer[qCaret], sizeof (Size));
    roData.m_Size   = nLiterals;
    roData.m_Memory = new Sym* [nLiterals];
    qCaret += sizeof (Size);

    String literalStrBuffer;
    Sym cSymBuff;
    for (Size iLiteral = Zero; iLiteral < nLiterals; iLiteral++) {
        cSymBuff = *(Sym*) &buffer[qCaret];
        while (cSymBuff != Zero) {
            literalStrBuffer += cSymBuff;
            cSymBuff = *(Sym*) &buffer[++qCaret];
        }
        qCaret++;
        roData.m_Memory[iLiteral] = GetNewLPCStr (literalStrBuffer);
        literalStrBuffer.clear ();
    }
    //Instructions processing
    Vector<VM::Instruction> aInstr;

    Size nInstr = *(Size*) (&buffer[qCaret]);
    qCaret += sizeof (Size);
    for (Size iInstr = Zero; iInstr < nInstr; iInstr++) {
        VM::OpCode opCode = *(VM::OpCode*) &buffer[qCaret++];
        VM::ValType valType = *(VM::ValType*) &buffer[qCaret++];
        Size wordSize = VM::ValUtility::SizeMap.at(valType);
        VM::Word word {};
        std::memcpy (&word, &buffer[qCaret], wordSize);
        VM::Instruction instr {
            .m_Op = opCode,
            .m_Val = VM::Val {
                .m_Type = valType,
                .m_Data = word
            }
        };
        aInstr.push_back (instr);
        qCaret += wordSize;
    }

    auto* instrBuffer = new VM::Instruction[aInstr.size ()];
    std::memcpy (instrBuffer, aInstr.data (), aInstr.size () * sizeof(VM::Instruction));

    asm_.m_Memory = instrBuffer,
    asm_.m_Size = aInstr.size ();
}

Undef TucanScript::Binary::BinaryBuilder::Build (
	const VM::ReadOnlyData& roData,
	const VM::Asm& asm_, 
	const String& outFilePath) {
    Size totalSize = sizeof (Sym);

    totalSize += sizeof (Size);
    for (Size iROStr = Zero; iROStr < roData.m_Size; iROStr++) {
        totalSize += NextWord (std::strlen(roData.m_Memory[iROStr]));
    }

    totalSize += sizeof (Size);
    for (Size iInstr = Zero; iInstr < asm_.m_Size; iInstr++) {
        const auto& instr = asm_.m_Memory[iInstr];
        totalSize += sizeof (VM::OpCode) + sizeof (VM::ValType);
        totalSize += VM::ValUtility::SizeMap.at (instr.m_Val.m_Type);
    }

    Sym* buffer = new Sym[totalSize];
    QWord qCaret = Zero;

    buffer[qCaret++] = VERIFICATION_HEADER;

    *(Size*)(buffer + qCaret) = roData.m_Size;
    qCaret += sizeof (Size);
    for (Size iROStr = Zero; iROStr < roData.m_Size; iROStr++) {
        Sym* str = roData.m_Memory[iROStr];
        Size strLength = std::strlen (str);
        std::memcpy (&buffer[qCaret], str, strLength);
        qCaret += strLength;
        buffer[qCaret++] = Zero;
    }

    *(Size*)(buffer + qCaret) = asm_.m_Size;
    qCaret += sizeof (Size);
    for (Size iInstr = Zero; iInstr < asm_.m_Size; iInstr++) {
        const auto& instr = asm_.m_Memory[iInstr];
        *(VM::OpCode*) &buffer[qCaret++] = instr.m_Op;
        *(VM::ValType*) &buffer[qCaret++] = instr.m_Val.m_Type;
        Size valSize = VM::ValUtility::SizeMap.at (instr.m_Val.m_Type);
        std::memcpy (buffer + qCaret, &instr.m_Val.m_Data, valSize);
        qCaret += valSize;
    }

    OFileStream outFileStream (outFilePath, std::ios::binary);
    if (!outFileStream) {
        LogErr ("Error opening file for writing: " << outFilePath);
        delete[] buffer;
        return;
    }

    outFileStream.write (buffer, (SInt64) totalSize);
    outFileStream.close ();
    delete[] buffer;
}