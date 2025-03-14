# Based upon: https://raw.githubusercontent.com/grpc/grpc/master/cmake/msvc_static_runtime.cmake
# Copyright 2017 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if(MSVC)
    option(EAR_PLUGINS_MSVC_STATIC_RUNTIME "Link with static msvc runtime libraries" ON)
    
    if(EAR_PLUGINS_MSVC_STATIC_RUNTIME)
        # switch from dynamic to static linking of msvcrt
        foreach(flag_var
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

            if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MD")
        endforeach(flag_var)
		
		# Despite the above, for some reason both FFTS and kissfft resort to /MD (despite also being static) without the following;
		add_compile_options(
			$<$<CONFIG:>:/MT>
			$<$<CONFIG:Debug>:/MTd>
			$<$<CONFIG:Release>:/MT>
			$<$<CONFIG:RelWithDebInfo>:/MT>
			$<$<CONFIG:MinSizeRel>:/MT>
		)
    endif()
endif()
