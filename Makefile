all:
	cc -Wall -Werror -Wextra -std=c++98 main.cpp HttpRequest.cpp -lstdc++ -o run

clean:
	rm run

fclean: clean

re: fclean all
