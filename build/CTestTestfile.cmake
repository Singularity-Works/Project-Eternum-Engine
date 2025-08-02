# CMake generated Testfile for 
# Source directory: C:/Users/nekoy/Desktop/Project-Eternum-Engine
# Build directory: C:/Users/nekoy/Desktop/Project-Eternum-Engine/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(EternumTests "C:/Users/nekoy/Desktop/Project-Eternum-Engine/build/Debug/eternum-tests.exe")
  set_tests_properties(EternumTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/nekoy/Desktop/Project-Eternum-Engine/CMakeLists.txt;21;add_test;C:/Users/nekoy/Desktop/Project-Eternum-Engine/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(EternumTests "C:/Users/nekoy/Desktop/Project-Eternum-Engine/build/Release/eternum-tests.exe")
  set_tests_properties(EternumTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/nekoy/Desktop/Project-Eternum-Engine/CMakeLists.txt;21;add_test;C:/Users/nekoy/Desktop/Project-Eternum-Engine/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(EternumTests "C:/Users/nekoy/Desktop/Project-Eternum-Engine/build/MinSizeRel/eternum-tests.exe")
  set_tests_properties(EternumTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/nekoy/Desktop/Project-Eternum-Engine/CMakeLists.txt;21;add_test;C:/Users/nekoy/Desktop/Project-Eternum-Engine/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(EternumTests "C:/Users/nekoy/Desktop/Project-Eternum-Engine/build/RelWithDebInfo/eternum-tests.exe")
  set_tests_properties(EternumTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/nekoy/Desktop/Project-Eternum-Engine/CMakeLists.txt;21;add_test;C:/Users/nekoy/Desktop/Project-Eternum-Engine/CMakeLists.txt;0;")
else()
  add_test(EternumTests NOT_AVAILABLE)
endif()
