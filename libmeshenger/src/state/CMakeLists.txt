add_library(libmeshenger-state
	state.cpp)

add_executable (test_state test.cpp)
target_link_libraries(test_state LINK_PUBLIC cryptopp ${LIBMESHENGER})
