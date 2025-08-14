# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror

# Target executable and object files
TARGET = ls
OBJ = ls.o

# Default target
all: libft_make printf_make $(TARGET)

# Link the object file to create the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) libft/libft.a

libft_make:
	cd libft && $(MAKE)
printf_make:
	cd ft_printf && $(MAKE)

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJ)
	cd ft_printf && $(MAKE) clean
	cd libft && $(MAKE) clean

fclean: clean
	rm -f $(TARGET) $(OBJ)
	cd libft && $(MAKE) fclean
	cd ft_printf && $(MAKE) fclean
# Phony targets
.PHONY: all clean libft_make
