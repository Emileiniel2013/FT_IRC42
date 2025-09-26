EXEC := ircserv

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++17

SRC_DIR		= 	src
SRC 		= 	$(wildcard $(SRC_DIR)/*.cpp)

OBJ_DIR		=	obj
OBJ			= 	$(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))


GREEN := \033[0;32m
NC := \033[0m

all: $(EXEC)

$(EXEC): $(OBJ)
	@echo "Linking $(EXEC)..."
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJ)
	@echo -e "$(GREEN)Build successful!$(NC)"

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up object files..."
	rm -rf $(OBJ_DIR)

fclean: clean
	@echo "Cleaning up $(EXEC)..."
	rm -f $(EXEC)

re: fclean all

.PHONY: all clean fclean re