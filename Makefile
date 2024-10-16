NAME = socket
SRC = main.cpp \
		socket.cpp \
		./request/RequestHandler.cpp \
		./request/Request.cpp \
		./response/Response.cpp
OBJ = $(SRC:.cpp=.o)
COMPILER = c++ -std=c++11
FLAGS = -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJ)
	$(COMPILER) $(FLAGS) -o $@ $^

%.o: %.cpp
	$(COMPILER) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
