cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME defi_db_export)

project($PROJECT_NAME)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(LIBRARY_NAME defi_db_export)

add_library(${LIBRARY_NAME} SHARED
  src/mysql_db.cpp
  src/defi_db_export.cpp
  src/export_utils.cpp
  src/daily_accumulator.cpp
  src/block_accumulator.cpp
)
target_include_directories(${LIBRARY_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/)

set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_include_directories(${LIBRARY_NAME} PUBLIC
  "/usr/include/mysql-cppconn-8/"
  ${HEADER_DIR})

target_link_libraries(${LIBRARY_NAME} PUBLIC
  mysqlcppconn)

set_target_properties(${LIBRARY_NAME} PROPERTIES PUBLIC_HEADER
  "${HEADER_DIR}/date.h;${HEADER_DIR}/export_utils.h;${HEADER_DIR}/mysql_db.h;${HEADER_DIR}/daily_accumulator.h;${HEADER_DIR}/defi_db_export.h;${HEADER_DIR}/block_accumulator.h")
install(TARGETS ${LIBRARY_NAME}
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${LIBRARY_NAME})

add_executable(defi_export_test
  test/defi_db_export.cpp)

target_link_libraries(defi_export_test PRIVATE
  gtest
  gtest_main
  pthread
  ${LIBRARY_NAME})

add_test(NAME DefiExportTest COMMAND defi_export_test)
enable_testing()

# patch defid: defid$(EXEEXT): $(defid_OBJECTS) $(defid_DEPENDENCIES) $(EXTRA_defid_DEPENDENCIES) 
#	@rm -f defid$(EXEEXT)
#	$(AM_V_CXXLD)$(defid_LINK) $(defid_OBJECTS) $(defid_LDADD) $(LIBS) -L/usr/local/lib -ldefi_db_export
# or: LIBS =  -L/usr/local/lib -ldefi_db_export