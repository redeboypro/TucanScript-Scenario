cmake_minimum_required (VERSION 3.8)
project ("TucanScript")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library (Graphics SHARED
			#Vulkan
			TucanScript/SystemModules/Graphics/Vulkan.h
			TucanScript/SystemModules/Graphics/Vulkan.cpp
			TucanScript/Utility.h
			TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp)

target_include_directories(Graphics PUBLIC
		TucanScript/ThirdParty/IncludeOSX
)

target_link_libraries(Graphics PUBLIC
		# Static libs
		/Users/redeboy/StaticLibraries/libglfw3.a
		/Users/redeboy/StaticLibraries/libMoltenVK.a

		# macOS frameworks (ОБЯЗАТЕЛЬНО в таком виде)
		"-framework Cocoa"
		"-framework IOKit"
		"-framework CoreVideo"
		"-framework CoreFoundation"
		"-framework Metal"
		"-framework QuartzCore"
		"-framework Foundation"
		"-framework IOSurface"
)

#Graphics dependencies

#Vulkan Test
#add_executable(VkTest
#	TucanScript/VkTest/main.cpp
#	TucanScript/SystemModules/Graphics/Vulkan.h
#	TucanScript/SystemModules/Graphics/Vulkan.cpp
#	TucanScript/ThirdParty/lodepng.h
#	TucanScript/ThirdParty/lodepng.cpp)
#target_link_libraries(VkTest PRIVATE  Graphics)
#End Vulkan Test

#target_include_directories(VkTest PUBLIC
#    $ENV{H_GLFW}
#   ${CMAKE_CURRENT_SOURCE_DIR}/TucanScript/SystemModules
#)
#find_package(Vulkan REQUIRED)

#target_link_libraries(VkTest PUBLIC
#	$ENV{LIB_GLFW}
#	Vulkan::Vulkan
#)

#End Graphics dependencies

add_library (IO SHARED
		TucanScript/Utility.h
		TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp
		TucanScript/SystemModules/IO/IO.h TucanScript/SystemModules/IO/IO.cpp)

add_library (XGUI SHARED
		TucanScript/Utility.h
		TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp
		TucanScript/SystemModules/GUI/XGUI.mm)

target_link_libraries(XGUI
		"-framework AppKit")

#add_library(Metal SHARED
#		TucanScript/Utility.h
#		TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp
#		TucanScript/SystemModules/Metal/Metal.cpp TucanScript/SystemModules/Metal/Metal.h)

#target_include_directories(Metal PUBLIC
#		TucanScript/ThirdParty
#)

#target_link_libraries(Metal
#		"-framework Metal"
#		"-framework MetalKit"
#		"-framework QuartzCore"
#		"-framework AppKit")

add_library(Networking SHARED
		TucanScript/SystemModules/Sockets/Networking.cpp TucanScript/SystemModules/Sockets/Networking.h
		TucanScript/Utility.h
		TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp)

add_library (Math SHARED
			TucanScript/SystemModules/Math/Math.h TucanScript/SystemModules/Math/Math.cpp
			TucanScript/Utility.h
			TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp)

add_library (Time SHARED
			TucanScript/SystemModules/Time/Time.h TucanScript/SystemModules/Time/Time.cpp
			TucanScript/Utility.h
			TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp)

add_library (Memory SHARED
			TucanScript/SystemModules/Memory/Memory.h TucanScript/SystemModules/Memory/Memory.cpp
			TucanScript/Utility.h
			TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp)

add_library (Coroutines SHARED
			TucanScript/SystemModules/Coroutines/Coroutines.h TucanScript/SystemModules/Coroutines/Coroutines.cpp
			TucanScript/Utility.h
			TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp)

add_executable (tbinc
			 TucanScript/BuildUtility/main.cpp
			 TucanScript/Utility.h
			 TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp 
			 TucanScript/Lexer.h TucanScript/Lexer.cpp
			 TucanScript/Compiler.h TucanScript/Compiler.cpp 
			 TucanScript/BinaryBuilder.h TucanScript/BinaryBuilder.cpp)

add_executable (trt
			 TucanScript/Runtime/main.cpp
			 TucanScript/Utility.h
			 TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp 
			 TucanScript/Lexer.h TucanScript/Lexer.cpp
			 TucanScript/Compiler.h TucanScript/Compiler.cpp 
			 TucanScript/BinaryBuilder.h TucanScript/BinaryBuilder.cpp)

add_library (TucanScript STATIC
			 TucanScript/Utility.h
			 TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp 
			 TucanScript/Lexer.h TucanScript/Lexer.cpp 
			 TucanScript/Compiler.h TucanScript/Compiler.cpp 
			 TucanScript/BinaryBuilder.h TucanScript/BinaryBuilder.cpp)

target_include_directories(TucanScript PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
