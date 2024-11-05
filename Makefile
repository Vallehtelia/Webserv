NAME = socket

REQUEST_DIR = ./request/
REQUEST_FILES = RequestHandler.cpp Request.cpp

RESPONSE_DIR = ./response/
RESPONSE_FILES = Response.cpp

SRC_DIR = ./
SRC_FILES = main.cpp socket.cpp

PARSE_DIR = ./parsing/
PARSE_FILES = ServerConfig.cpp LocationConfig.cpp parseConfFile.cpp

SRC += $(addprefix $(SRC_DIR), $(SRC_FILES))
SRC += $(addprefix $(REQUEST_DIR), $(REQUEST_FILES))
SRC += $(addprefix $(RESPONSE_DIR), $(RESPONSE_FILES))
SRC += $(addprefix $(PARSE_DIR), $(PARSE_FILES))


OBJ = $(SRC:.cpp=.o)
COMPILER = c++ -std=c++17
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
