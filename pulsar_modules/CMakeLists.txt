cmake_minimum_required(VERSION 3.0)
find_package(pulsar REQUIRED)

# Common flags for standard modules
# some useless remarks
#  1418  : external function definition with no prior declaration
#  1419  : external declaration in primary source file (happens in boost files)
#   383  : value copied to temporary, reference to temporary used (generally ok in c++)
#   981  : operands are evaluated in unspecified order (generally ok in c++)
#   869  : parameter "XXX" was never referenced.
#          This is due to a bug in Intel before Intel 16 that causes this
#          remark to be printed for variadic templates
# 11074  : Inlining inhibited by limit max-size (etc)
# 11076  : To get full report use -qopt-report=4 -qopt-report-phase ipo
#   444  : destructor for base class .. not virtual (even warns for classes without virtual functions)
    
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    foreach(a_flag "-std=c++11" "-w3" "-restrict;-DRESTRICT=restrict" "-wd1418"
        "-wd1419" "-wd383" "-wd981" "-wd869" "-wd11074" "-wd11076" "-wd444")
        list(APPEND PULSAR_CXX_STRICT_FLAGS ${a_flag})
    endforeach()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    foreach(a_flag "-std=c++11" "-DRESTRICT=__restrict__" 
        "-Wall;-Wextra;-pedantic" "-Wfloat-equal;-Wshadow" "-Wswitch-default"
        "-Wconversion")
       list(APPEND PULSAR_CXX_STRICT_FLAGS ${a_flag})
    endforeach()
endif()

#########################################
# Includes, compile flags, and linking
#########################################
# We need to add the parent of this directory
get_filename_component(PULSAR_STANDARDMODULES_PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
include_directories(${PULSAR_STANDARDMODULES_PARENT_DIR})


#################################################
# Obtain all the sources of the core submodules
#################################################
function(MakeFullPath FILELIST PREFIX)
   set(LV "")
   foreach(F ${ARGN})
      list(APPEND LV "${PREFIX}/${F}")
   endforeach(F)
   SET(${FILELIST} "${LV}" PARENT_SCOPE)
endfunction(MakeFullPath)

foreach(sub_dir common system_fragmenters methods integrals)
    add_subdirectory(${sub_dir})
    string(TOUPPER ${sub_dir} SUB_DIR)
    MakeFullPath(PULSAR_${SUB_DIR}_FILES_FULL ${sub_dir} ${PULSAR_${SUB_DIR}_SRC})
    list(APPEND PULSAR_MODULE_FILE_LIST ${PULSAR_${SUB_DIR}_FILES_FULL})
endforeach()
#add_subdirectory(TestModules) #RMR-Should these be moved to core?

add_library(pulsar_modules MODULE ${PULSAR_MODULE_FILE_LIST} creator.cpp)
target_compile_options(pulsar_modules PRIVATE ${PULSAR_CXX_STRICT_FLAGS})
target_include_directories(pulsar_modules PRIVATE pulsar)
target_link_libraries(pulsar_modules PRIVATE pulsar)

install(TARGETS pulsar_modules
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/pulsar_modules
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" 
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        FILES_MATCHING PATTERN "*.py"
)
