NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g3


#===============================================================================#
#=============================[ HEADERS ]========================================#
#===============================================================================#


HDRS_PATH = includes/
HDR_NAME = $(shell find $(HDRS_PATH) -name "*.hpp")
HDR_INCLUDE = -I $(HDRS_PATH)


#===============================================================================#
#=============================[ SOURCES ]========================================#
#===============================================================================#


SRCS_PATH = sources/
SRCS_RAW_ALL = $(shell find $(SRCS_PATH) -type f -name "*.cpp")
SRCS_ALL = $(patsubst sources/%,%,$(SRCS_RAW_ALL))


#===============================================================================#
#=============================[ OBJECTS ]========================================#
#===============================================================================#


OBJS_PATH = objs/
OBJS_NAME = $(SRCS_ALL:.cpp=.o)
OBJS = $(addprefix $(OBJS_PATH), $(OBJS_NAME))


#===============================================================================#
#=============================[ COLORS ]========================================#
#===============================================================================#


RESET = \033[0m
BOLD = \033[1m
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
ROSE = \033[35m


#===============================================================================#
#=============================[ RULES ]========================================#
#===============================================================================#


RM = rm -rf
all: project $(NAME) $(HDR_NAME)


project:
	@echo "$(BLUE)====================================$(RESET)"
	@echo "\n\t[ $(BLUE)$(BOLD)~~~~ PROJECT ~~~~$(RESET)]\n"


$(OBJS_PATH):
	@mkdir -p $(OBJS_PATH)
	@echo "\t[ $(GREEN)✓$(RESET) ] $(OBJS_PATH) directory done \n"


$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp $(HDR_NAME)
	@$(CC) $(CFLAGS) $(HDR_INCLUDE) -o $@ -c $<
	@echo "\t[ $(GREEN)✓$(RESET) ] $@ object"


$(NAME): $(OBJS_PATH) $(OBJS) $(HDR_NAME)
	@echo "$(SRCS_ALL)"
	@$(CC) $(CFLAGS) $(OBJS) $(HDR_INCLUDE) -o $@

	@echo "$(GREEN)\|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ $(RESET)"
	@echo "\t [ $(GREEN)✓$(RESET) ] $(BOLD)PROJECT IS READY $(RESET)[ $(GREEN)✓$(RESET) ] "
	@echo "$(GREEN)/|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ $(RESET)"


clean:
	@echo "\t[ $(RED)$(BOLD)=== CLEANING === $(RESET)]"
	@echo "$(RED)====================================$(RESET)"
	@echo "\t[ $(RED)✗$(RESET) ] $(OBJS)"
	@$(RM) $(OBJS) $(OBJS_PATH)


fclean: clean tclean
	@$(RM) $(NAME)
	@echo "\t[ $(RED)✗$(RESET) ] $(NAME) executable"


re: fclean all


test: all
	@docker-compose -f ./test/docker-compose.yml up -d --build 2>/dev/null
	@(>/dev/null 2>/dev/null ./$(NAME) &)
	@zsh test/test.sh
	@docker-compose -f ./test/docker-compose.yml down 2>/dev/null
	@PID=$$(ps -ax | grep -F ./$(NAME) | grep -v "grep" | awk '{print $$1}')
	@if [ -n "$${PID}" ] ; then kill $${PID}; fi


tclean:
	@docker-compose -f ./test/docker-compose.yml down 2>/dev/null
	@PID=$$(ps -ax | grep -F "./$(NAME)" | grep -v "grep" | awk '{print $$1}')
	@if [ -n "$${PID}" ] ; then kill $${PID}; fi


.PHONY: all clean fclean re