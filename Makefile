#===============================================================================#
#=============================[ PROJECT ]========================================#
#===============================================================================#

NAME = webserv
CC = c++
CXXFLAGS = -MMD -MP -Wall -Wextra -Werror -std=c++98


#===============================================================================#
#=============================[ HEADERS ]========================================#
#===============================================================================#


HDRS_PATH = includes/
HDR_NAME = $(shell find $(HDRS_PATH) -name "*.hpp")
HDR_INCLUDE = $(shell find $(HDRS_PATH) -type d -exec echo -I{} \;)


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
#=============================[ DEBUG ]========================================#
#===============================================================================#

DEBUG_LEVEL ?= 0

ifeq ($(DEBUG_LEVEL),1)
CXXFLAGS += -DDEBUG_LEVEL=1 -g3
endif

ifeq ($(DEBUG_LEVEL),2)
CXXFLAGS += -DDEBUG_LEVEL=2 -g3
endif


#===============================================================================#
#=============================[ RULES ]========================================#
#===============================================================================#


RM = rm -rf

LAST_CXXFLAGS := $(shell cat .last_build_flags 2>/dev/null)
CURRENT_CXXFLAGS := $(CXXFLAGS)

all: project $(NAME) $(HDR_NAME)
	@if [ "$(LAST_CXXFLAGS)" != "$(CURRENT_CXXFLAGS)" ]; then \
		$(MAKE) clean; \
		$(MAKE) $(NAME); \
	fi
	@echo "$(CURRENT_CXXFLAGS)" > .last_build_flags

valgrind: DEBUG_LEVEL = 2
valgrind: all 
	valgrind --leak-check=full --trace-children=yes --track-origins=yes ./webserv

project:
	@echo "$(BLUE)====================================$(RESET)"
	@echo "\n\t[ $(BLUE)$(BOLD)~~~~ PROJECT ~~~~$(RESET)]\n"


$(OBJS_PATH):
	@mkdir -p $(OBJS_PATH)
	@echo "\t[ $(GREEN)✓$(RESET) ] $(OBJS_PATH) directory done \n"


$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp $(HDR_NAME)
	@mkdir -p $(dir $@)
	@$(CC) $(CXXFLAGS) $(HDR_INCLUDE) -o $@ -c $<
	@echo "\t[ $(GREEN)✓$(RESET) ] $@ object"


$(NAME): $(OBJS_PATH) $(OBJS) $(HDR_NAME)
	@$(CC) $(CXXFLAGS) $(OBJS) $(HDR_INCLUDE) -o $@
	@echo "\t[ $(GREEN)✓$(RESET) ] $(NAME) executable"
	@echo "\t\t       $(ROSE)  "
	@echo "\t\t       $(ROSE).████:   ████████.        "
	@echo "\t\t       $(ROSE)████:███████████████.      "
	@echo "\t\t       $(ROSE)███:█████████████████:     "
	@echo "\t\t       $(ROSE)██:███████████████████:    "
	@echo "\t\t       $(ROSE)█:███████$(BLUE)▒▒$(ROSE)███$(BLUE)▒▒$(ROSE)██████    "
	@echo "\t\t       $(ROSE) ████████$(BLUE)▒▒▒$(ROSE)██$(BLUE)▒▒▒$(ROSE)███████   "
	@echo "\t\t       $(ROSE) ████████$(BLUE)▒▒$(ROSE)███$(BLUE)▒▒$(ROSE)█████████  "
	@echo "\t\t       $(ROSE) ██████████$(RED)▞▞▞$(ROSE)████████████:"
	@echo "\t\t       $(ROSE) ██████████$(RED)▞▞▞$(ROSE)█████████████"
	@echo "\t\t       $(ROSE)   ███████████████████ ████"
	@echo "\t\t       $(ROSE)    █████████████████   ███"
	@echo "\t\t       $(ROSE)      █████████████   ████ "
	@echo "\t\t       $(RED) ▁▁▁▁▁▁▁▁▁$(ROSE)█████$(RED)▁▁▁▁▁▁▁▁▁   "
	@echo "\t\t       $(RED)██████████████████████████. "
	@echo "\t\t       $(RED)██████████████████████████ "
	@echo "\t\t       $(RED)███████████   ████████████ "
	@echo "\t\t       $(RED)████████       █████████ "
	@echo "\t\t       $(RED) ██████           ██████.  "


	@echo " ▄▄▄·▄▄▌  ▄▄▄ . ▄▄▄· .▄▄ · ▄▄▄ .    ▄ •▄ ▪  ▄▄▌  ▄▄▌      • ▌ ▄ ·. ▄▄▄ ."
	@echo "▐█ ▄███•  ▀▄.▀·▐█ ▀█ ▐█ ▀. ▀▄.▀·    █▌▄▌▪██ ██•  ██•      ·██ ▐███▪▀▄.▀·"
	@echo " ██▀·██▪  ▐▀▀▪▄▄█▀▀█ ▄▀▀▀█▄▐▀▀▪▄    ▐▀▀▄·▐█·██▪  ██▪      ▐█ ▌▐▌▐█·▐▀▀▪▄"
	@echo "▐█▪·•▐█▌▐▌▐█▄▄▌▐█ ▪▐▌▐█▄▪▐█▐█▄▄▌    ▐█.█▌▐█▌▐█▌▐▌▐█▌▐▌    ██ ██▌▐█▌▐█▄▄▌"
	@echo ".▀   .▀▀▀  ▀▀▀  ▀  ▀  ▀▀▀▀  ▀▀▀     ·▀  ▀▀▀▀.▀▀▀ .▀▀▀     ▀▀  █▪▀▀▀ ▀▀▀ "


	@echo "$(GREEN)\|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ \|/ $(RESET)"
	@echo "\t [ $(GREEN)✓$(RESET) ] $(BOLD)PROJECT IS READY $(RESET)[ $(GREEN)✓$(RESET) ] "
	@echo "$(GREEN)/|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ /|\ $(RESET)"




clean:
	@echo "\t[ $(RED)$(BOLD)=== CLEANING === $(RESET)]"
	@echo "$(RED)====================================$(RESET)"
	@echo "\t[ $(RED)✗$(RESET) ] $(OBJS)"
	@$(RM) $(OBJS) $(OBJS_PATH)


fclean: clean
	@$(RM) $(NAME)
	@echo "\t[ $(RED)✗$(RESET) ] $(NAME) executable"

re: fclean all

.PHONY: all clean fclean re  valgrind project 

-include $(OBJS:.o=.d)
