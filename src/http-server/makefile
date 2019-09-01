obj = main.o httpserver.o httparse.o demo_func.o
target = app
$(target) : $(obj)
	g++ $(obj) -o $(target)
%.o : %.cpp
	g++ -c $< -o $@ -Wall -std=c++11
clean:
	rm *.o $(target)
