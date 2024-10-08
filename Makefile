# Compiler
CC = gcc

# Compiler flags
CFLAGS =   -Wall -ansi -pedantic 

# Include directories
INCLUDES = -I./header_files

# Source directory
SRCDIR = source_files

# Object directory
OBJDIR = object_files

# Output directory
OUTDIR = output

# Source files
SRCS = $(wildcard $(SRCDIR)/*.c)

# Object files
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)


# Executable name
EXEC = assembler

# Default target
all: $(EXEC)

# Link object files to create executable
$(EXEC): $(OBJS)
	@echo "Linking object files to create executable..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^
	@echo "Build complete."

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Create the object directory if it doesn't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Create the output directory if it doesn't exist
$(OUTDIR):
	mkdir -p $(OUTDIR)	


# Clean up
clean:
	rm -f $(OBJDIR)/*.o $(EXEC) 
	rm -f $(OUTDIR)/*.am  $(OUTDIR)/*.ent  $(OUTDIR)/*.ext  $(OUTDIR)/*.ob 
	@echo "Clean complete."

reset:
	rm -f $(OUTDIR)/*.am $(OUTDIR)/*.ent $(OUTDIR)/*.ext $(OUTDIR)/*.ob 
	@echo "Reset complete"	

# Phony targets
.PHONY: all clean
