NAME = socket

REQUEST_DIR = ./request/
REQUEST_FILES = RequestHandler.cpp Request.cpp cgi_request.cpp SessionManager.cpp

RESPONSE_DIR = ./response/
RESPONSE_FILES = Response.cpp

SRC_DIR = ./
SRC_FILES = main.cpp utils.cpp

SOCKET_DIR = ./sockets/
SOCKET_FILES = socket.cpp

EPOLL_DIR = ./epoll/
EPOLL_FILES = epoll.cpp

EVENT_DIR = ./events/
EVENT_FILES = eventLoop.cpp

PARSE_DIR = ./parsing/
PARSE_FILES = ServerConfig.cpp LocationConfig.cpp parseConfFile.cpp ConfigValidator.cpp config.cpp


SRC += $(addprefix $(SRC_DIR), $(SRC_FILES))
SRC += $(addprefix $(REQUEST_DIR), $(REQUEST_FILES))
SRC += $(addprefix $(RESPONSE_DIR), $(RESPONSE_FILES))
SRC += $(addprefix $(SOCKET_DIR), $(SOCKET_FILES))
SRC += $(addprefix $(EPOLL_DIR), $(EPOLL_FILES))
SRC += $(addprefix $(EVENT_DIR), $(EVENT_FILES))
SRC += $(addprefix $(PARSE_DIR), $(PARSE_FILES))
SRC += $(addprefix $(CGI_DIR), $(CGI_FILES))

OBJ = $(SRC:.cpp=.o)
COMPILER = c++ -std=c++17
CFLAGS = -Wall -Wextra -Werror # -g -O0
LFLAGS =

all: $(NAME)

$(NAME): $(OBJ)
	$(COMPILER) $(CFLAGS) -o $@ $^ $(LFLAGS)

%.o: %.cpp
	$(COMPILER) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
