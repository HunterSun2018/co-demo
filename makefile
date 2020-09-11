demo : src/main.cpp src/co_helper.hpp
	g++-10 -g -std=c++20 -fcoroutines -Wall src/main.cpp -lpthread -o demo

clang : src/main.cpp
	clang++ -std=c++2a -fcoroutines-ts -stdlib=libc++ awaitor.cpp -lpthread -o demo
	
clean :
	rm demo