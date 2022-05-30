// File: mopsolver.c
//
// Description: Performs the mopsolver program with various input
//
// @author      Jack Schneider, jas1727
//

#define _DEFAULT_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

int queue_length;
int neighbors_list_length;
int explored_length;
int path_length;
struct coordinates *queue;
struct coordinates *neighbors_list;
struct coordinates *explored;

// Structure for coordinates
// @param x_loc the x coordinate
// @param y_loc the y coordinate
struct coordinates{
	int x_loc;
	int y_loc;
};

// Adds an element to the end of a list
// @param length_char char determining which length to use
// @param new_coordinate the given coordinate to be inserted into the queue
void addToQueue(char length_char, struct coordinates new_coordinate) {
	if(length_char == 'q') {
		queue_length++;
		queue = realloc(queue, queue_length * sizeof(struct coordinates));
	        queue[queue_length - 1] = new_coordinate;

	} else if(length_char == 'n') {
		neighbors_list_length++;
		neighbors_list = realloc(neighbors_list, neighbors_list_length * sizeof(struct coordinates));
	        neighbors_list[neighbors_list_length - 1] = new_coordinate;

	} else if(length_char == 'e') {
		explored_length++;
		explored = realloc(explored, explored_length * sizeof(struct coordinates));
	        explored[explored_length - 1] = new_coordinate;
	}
}

// Removes an element from the beginning of a queue and returns it
// @param length_char char determining which length to use
// @return the removed coordinate
struct coordinates removeFromQueue(char length_char) {
	struct coordinates removed_coordinate;
	struct coordinates *edited_queue;
	if(length_char == 'q') {
                queue_length--;
		removed_coordinate = queue[0];
   	        edited_queue = malloc(queue_length * sizeof(struct coordinates));
        	for(int x = 1; x < queue_length + 1; x++) {
                	edited_queue[x-1] = queue[x];
        	}
		free(queue);
        	queue = realloc(edited_queue, queue_length * sizeof(struct coordinates));
        	for(int x = 0; x < queue_length; x++) {
                	queue[x] = edited_queue[x];
        	}
		if(queue_length != 0) {
			free(edited_queue);
		}
        } else if(length_char == 'n') {
                neighbors_list_length--;
                removed_coordinate = neighbors_list[0];
                edited_queue = malloc(neighbors_list_length * sizeof(struct coordinates));
                for(int x = 1; x < neighbors_list_length + 1; x++) {
                        edited_queue[x-1] = neighbors_list[x];
                }
		free(neighbors_list);
                neighbors_list = realloc(edited_queue, neighbors_list_length * sizeof(struct coordinates));
                for(int x = 0; x < neighbors_list_length; x++) {
                        neighbors_list[x] = edited_queue[x];
                }
		if(neighbors_list_length != 0) {
			free(edited_queue);
		}
        } else if(length_char == 'e') {
                explored_length--;
                removed_coordinate = explored[0];
                edited_queue = malloc(explored_length * sizeof(struct coordinates));
                for(int x = 1; x < explored_length + 1; x++) {
                        edited_queue[x-1] = explored[x];
                }
		free(explored);
                explored = realloc(edited_queue, explored_length * sizeof(struct coordinates));
                for(int x = 0; x < explored_length; x++) {
                        explored[x] = edited_queue[x];
                }
		if(explored_length != 0) {
			free(edited_queue);
		}
        }
	return removed_coordinate;
}

// Prints the intended usage message depending if there is an invalid flag
// @param invalid_flag_present the status of whether there is an invalid flag
void printUsageMessage(int invalid_flag_present) {
	if (invalid_flag_present) {
		fprintf(stderr, "Usage:\nmopsolver [-hdsp] [-i INFILE] [-o OUTFILE]\n"
                        "Options:\n  -h          Print usage and options"
                        " list to stdout only.    (Default: off)\n"
                        "  -d          Pretty-print (display) the maze after reading."
                        "  (Default: off)\n  -s          Print length of shortest path"
                        " or \'No solution\'. (Default: off)\n  -p          "
                        "Pretty-print maze with the path, if one exists. (Default: off)\n"
                        "  -i infile   Read maze from infile.                          "
                        "(Default: stdin)\n  -o outfile  Write all output to outfile."
                        "                   (Default: stdout)\n");
	} else {
		printf("Usage:\nmopsolver [-hdsp] [-i INFILE] [-o OUTFILE]\n"
                        "Options:\n  -h          Print usage and options"
                        " list to stdout only.    (Default: off)\n"
                        "  -d          Pretty-print (display) the maze after reading."
                        "  (Default: off)\n  -s          Print length of shortest path"
                        " or \'No solution\'. (Default: off)\n  -p          "
                        "Pretty-print maze with the path, if one exists. (Default: off)\n"
                        "  -i infile   Read maze from infile.                          "
                        "(Default: stdin)\n  -o outfile  Write all output to outfile."
                        "                   (Default: stdout)\n");
	}
}

// The main function for performing the mopsolver program for various inpout
// @param argc the number of arguments
// @param argv an array of the arguments
// @return EXIT_SUCCESS returns if the program runs as intended
int main( int argc, char *argv[]) {

	int input_from_file = 0; // Boolean initialized as false to take input from file
	int output_to_file = 0; // Boolean initialized as false to print output to file
	int p_print_init = 0; // Boolean inititialized as false to print the initial maze
	int print_steps = 0; // Boolean initialized as false to print the steps til solution
	int p_print_w_path = 0; // Boolean initialized as false to print maze with path
	FILE *input_stream = stdin;
	FILE *output_stream;

	int opt;
	while ( (opt = getopt( argc, argv, "hdspi:o:") ) != -1 ) {
		switch ( opt ) {
			case 'i' :
				input_from_file = 1;
				input_stream = fopen(optarg, "r");
				break;
			case 'h' :
				printUsageMessage(0);
				return EXIT_SUCCESS;
				break;
			case 'd' :
				p_print_init = 1;
				break;
			case 's' :
				print_steps = 1;
				break;
			case 'p' :
				p_print_w_path = 1;
				break;
			case 'o' :
				output_to_file = 1;
				output_stream = fopen(optarg, "a");
				break;
			default:
				printUsageMessage(1);
				return EXIT_SUCCESS;
		}
	}


	char **maze_2d = malloc(sizeof(char*));
        maze_2d[0] = malloc(sizeof(char));
	int maze_row_length;
	int maze_column_length;

	// apply input to maze from either a file or standard input
	int x_pos = 0;
        int y_pos = 0;
        char i_char;
        do {
                i_char = fgetc(input_stream);
                if(i_char != EOF) {
                        if(i_char != ' ') {
                                if(i_char != '\n') {
                                        if(y_pos == 0 && x_pos != 0) {
                                                maze_2d = realloc(maze_2d, (1 + x_pos) * sizeof(char*));
                                                maze_2d[x_pos] = malloc(sizeof(char));
                                        } else if(y_pos != 0) {
                                                maze_2d[x_pos] = realloc(maze_2d[x_pos], (y_pos + 1) * sizeof(char));
                                        }
                                        maze_2d[x_pos][y_pos] = i_char;
                                        x_pos++;
				} else {
                                        maze_row_length = x_pos;
                                        x_pos = 0;
                                        y_pos++;
                                }
                        }
                }
        } while(i_char != EOF);
        maze_column_length = y_pos;
	if(input_from_file) {
		fclose(input_stream);
	}

	int pres_row_length = 3 + 2*maze_row_length;
	int pres_column_length = 2 + maze_column_length;
	char presentation_maze[pres_row_length][pres_column_length];

	// initializes the presentation maze
	if (p_print_init || p_print_w_path) {
		for(int x = 0; x < pres_row_length; x++) {
			for(int y = 0; y < pres_column_length; y++) {
				presentation_maze[x][y] = ' ';
				if(y == 0 || y == pres_column_length - 1) {
					presentation_maze[x][y] = '-';
				}
				if(x == 0 || x == pres_row_length - 1) {
					presentation_maze[x][y] = '|';
				}
				if((x == 0 && y == 1) || (x == pres_row_length - 1 && y == pres_column_length - 2)) {
					presentation_maze[x][y] = ' ';
				}
			}
		}
		for(int x = 0; x < maze_row_length; x++) {
			for(int y = 0; y < maze_column_length; y++) {
				if(maze_2d[x][y] == '0') {
					presentation_maze[2 + (2*x)][y + 1] = '.';
				} else {
					presentation_maze[2 + (2*x)][y + 1] = '#';
				}
			}
		}
	}

	// performs the BFS search to find the path of coordinates and the number of coordinates
	struct coordinates init_coordinate;
	init_coordinate.x_loc = 0;
	init_coordinate.y_loc = 0;
	queue = malloc(0);
        queue_length = 0;
	neighbors_list = malloc(0);
	neighbors_list_length = 0;
	explored = malloc(0);
	explored_length = 0;
	int solution_present; // Boolean representing the presence of a solution in the maze

	if(maze_2d[0][0] == '1') {
		solution_present = 0;
	} else {
		addToQueue('e', init_coordinate);
		addToQueue('q', init_coordinate);
		while(queue_length != 0) {
			struct coordinates current_coordinate = removeFromQueue('q');
			if(current_coordinate.x_loc == maze_row_length -1 && current_coordinate.y_loc == maze_column_length -1) {
				addToQueue('q', current_coordinate);
				solution_present = 1;
				break;
			}

			neighbors_list_length = 0;
			neighbors_list = realloc(neighbors_list, 0);
			// check up
			if(current_coordinate.y_loc != 0 && maze_2d[current_coordinate.x_loc][current_coordinate.y_loc -1] == '0') {
				struct coordinates new_coordinate;
				new_coordinate.x_loc = current_coordinate.x_loc;
				new_coordinate.y_loc = current_coordinate.y_loc -1;
				addToQueue('n', new_coordinate);
			}

			// check down
			if(current_coordinate.y_loc != maze_column_length -1 && maze_2d[current_coordinate.x_loc][current_coordinate.y_loc +1] == '0') {
				struct coordinates new_coordinate;
				new_coordinate.x_loc = current_coordinate.x_loc;
                                new_coordinate.y_loc = current_coordinate.y_loc +1;
				addToQueue('n', new_coordinate);
			}
			
			// check left
			if(current_coordinate.x_loc != 0 && maze_2d[current_coordinate.x_loc -1][current_coordinate.y_loc] == '0') {
				struct coordinates new_coordinate;
                                new_coordinate.x_loc = current_coordinate.x_loc -1;
                                new_coordinate.y_loc = current_coordinate.y_loc;
				addToQueue('n', new_coordinate);
			}
			
			// check right
			if(current_coordinate.x_loc != maze_row_length -1 && maze_2d[current_coordinate.x_loc +1][current_coordinate.y_loc] == '0') {
				struct coordinates new_coordinate;
                                new_coordinate.x_loc = current_coordinate.x_loc +1;
                                new_coordinate.y_loc = current_coordinate.y_loc;
				addToQueue('n', new_coordinate);
			}

			for(int x = 0; x < neighbors_list_length; x++) {
				int neighbor_explored = 0; // boolean of the state of whether the current neighbor has been explored
				for(int i = 0; i < explored_length; i++) {
					if(explored[i].x_loc == neighbors_list[x].x_loc && explored[i].y_loc == neighbors_list[x].y_loc) {
						neighbor_explored = 1;
					}
				}
				if(neighbor_explored == 0) {
					addToQueue('e', neighbors_list[x]);
					addToQueue('q', neighbors_list[x]);
				}
			}
		}
		if(queue_length == 0) {
			solution_present = 0;
		}
	}
	free(neighbors_list);
	free(explored);

	// prints to either std-output or a file
	if(output_to_file) {
		if(p_print_init) {
        	        // print presentation maze to FILE
			for(int y = 0; y < pres_column_length; y++) {
                                for(int x = 0; x < pres_row_length; x++) {
					fputc(presentation_maze[x][y], output_stream);
                                }
                                fputc('\n', output_stream);
                        }
 	        }
        	if (print_steps) {
                	// print num of steps, equaling length of bfs queue to FILE
			if(solution_present) {
				fprintf(output_stream, "Solution in %d steps.\n", queue_length);
			} else {
				fputs("No solution.\n", output_stream);
			}
        	}
        	if (p_print_w_path) {
                	// alters presentation maze based on path and prints to FILE
			if(solution_present) {
				for(int x = 0; x < queue_length; x++) {
                                        presentation_maze[(2 * queue[x].x_loc) + 2][(queue[x].y_loc) + 1] = '+';
                                }
			}
			for(int y = 0; y < pres_column_length; y++) {
                                for(int x = 0; x < pres_row_length; x++) {
                                        fputc(presentation_maze[x][y], output_stream);
                                }
                                fputc('\n', output_stream);
                        }
        	}
		fclose(output_stream);
	} else {
		if(p_print_init) {
                        // print presentation maze to STD-OUT
			for(int y = 0; y < pres_column_length; y++) {
                                for(int x = 0; x < pres_row_length; x++) {
                                        printf("%c", presentation_maze[x][y]);
                                }
                                printf("\n");
                        }
                }
                if (print_steps) {
                        // print num of steps, equaling length of bfs queue to STD-OUT
			if(solution_present) {
				printf("Solution in %d steps.\n", queue_length);
			} else {
				printf("No solution.\n");
			}
                }
                if (p_print_w_path) {
                        // alters presentation maze based on path and prints to STD-OUT
			if(solution_present) {
				for(int x = 0; x < queue_length; x++) {
					presentation_maze[(2 * queue[x].x_loc) + 2][(queue[x].y_loc) + 1] = '+';
				}
			}
			for(int y = 0; y < pres_column_length; y++) {
                                for(int x = 0; x < pres_row_length; x++) {
                                        printf("%c", presentation_maze[x][y]);
                                }
                                printf("\n");
                        }

                }
	}

	// free the necessary variables
	for(int x = 0; x < maze_row_length; x++) {
		free(maze_2d[x]);
	}
	free(maze_2d);
	free(queue);

	return EXIT_SUCCESS;
}
