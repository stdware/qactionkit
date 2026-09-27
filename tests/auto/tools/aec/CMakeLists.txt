# Tests of the manifests that AEC accepts and rejects. Written in Python, because each case checks
# both the exit status and the message on the standard error, which a script expresses more
# directly than CTest properties. See test_aec.py for the format of the cases.
#
# Set Python3_EXECUTABLE if the first Python found is unsuitable, which on Windows may be the
# Microsoft Store placeholder on PATH.
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_test(NAME tst_aec
    COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/test_aec.py
        $<TARGET_FILE:qak_aec> ${CMAKE_CURRENT_SOURCE_DIR}/cases
)
