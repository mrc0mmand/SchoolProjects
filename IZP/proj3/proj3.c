/*
 * @brief 	Projekt 3 - Pruchod bludistem
 * @author 	Frantisek Sumsal (xsumsa01@stud.fit.vutbr.cz)
 * 			VUT 1 BIT (BIB - 37)
 * @date	28.11.2014
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

/** Data types **/

/**
 * @brief 	Structure for saving command line arguments
 * 			and their appropriate functions.
 */
struct path_arg_t {
	const char *name;	/**< Argument string */
	int (*f)();			/**< Function pointer */
};

/**
 * @brief Bitmap structure.
 */
typedef struct {
	int rows;		/**< Number of rows */
	int cols;		/**< Number of columns */
	char *cells;	/**< Map array */
} Map;

/**
 * @brief Structure for saving border data.
 */
typedef struct {
	char b;		/**< Border */
	char d;		/**< Direction */
} border_t;

/**
 * @brief Stucture for saving coordinations data.
 */
typedef struct {
	int xp;		/**< Parent's x coord */
	int yp;		/**< Parent's y coord */
	int x;		/**< x coord */
	int y;		/**< y coord */
} coords_t;

/**
 * @brief Queue implementation.
 */
typedef struct {
	int size;
	coords_t *buffer;
	int start;
	int end;
} queue_t;

/**
 * @brief Error codes.
 */
enum ecodes {
	E_OK = 0,	/* Everything is OK */
	E_IVAL,		/* Invalid value */
	E_IRANGE,	/* Invalid range */
	E_IARG,		/* Invalid argument */
	E_IFILE,	/* Invalid file */
	E_OOM, 		/* Out of memory */
	E_QEMPTY,	/* Empty queue */
	E_OTHER		/* Other error */
};

/**
 * @brief Border constants.
 */
enum borders {
	B_LEFT = 0x1,	/* Left border is a wall*/
	B_RIGHT = 0x2,	/* Right border is a wall*/
	B_TOPDOWN = 0x4	/* Top or down border is a wall*/
};

/**
 * @brief Direction constants/
 */
enum direction_rules {
	D_LEFT = 0,
	D_RIGHT
};

/** Function prototypes **/

/**
 * @brief Malloc with error checking.
 * 
 * @param size Block size to allocate.
 * @return Allocated block of memory.
 */
void *emalloc(size_t size);

/**
 * @brief Returns brief description of error code.
 * 
 * @param ec Error code (see enum ecodes).
 * @return String with brief description.
 */
char *ecode_string(int ec);

/**
 * @brief Initializes map structure.
 * @details Sets map strucutre variables for rows and columns
 * 			and allocates required memory for cells array.
 * 
 * @param map Pointer to Map structure.
 * @param rows Row count.
 * @param cols Column count.
 * @return Error code (see enum ecodes).
 */
int init_map(Map *map, int rows, int cols);

/**
 * @brief Releases memory of cells array in given map structure.
 * 
 * @param map Valid pointer to Map structure.
 */
void free_map(Map *map);

/**
 * @brief Lodas map from file into Map structure.
 * 
 * @param file Name of file with map data.
 * @param map Valid pointer to Map structure.
 * @param test If true, function will clean map structure before returning.	
 * @return Error code (see enum ecodes).
 */	
int load_map(const char *file, Map *map, bool test);

/**
 * @brief Checks, if map fields' values correspond to their shared borders.
 * 
 * @param map Valid pointer to Map structure.
 * @return Error code (see enum ecodes).
 */
int check_shared_borders(Map *map);

/**
 * @brief Checks, if border on given coordinations is a wall.
 * 
 * @param map Valid pointer to Map structure.
 * @param x Row (indexed from 0).
 * @param y Column (indexed from 0).
 * @param border Type of border (see enum borders).
 * @return True, if border on given coordinations is a wall, false otherwise.
 */
bool isborder(Map *map, int x, int y, int border);

/**
 * @brief Determines border to follow from starting coordinations.
 * 
 * @param map Valid pointer to Map structure.
 * @param x Row (indexed from 1).
 * @param y Column (indexed from 1).
 * @param leftright Direction (see enum direction_rules).
 * @return Border to follow (see enum borders).
 */
int start_border(Map *map, int x, int y, int leftright);

/**
 * @brief Rotates three-bit bordder by number of given places and in given direction.
 * @details Uses bitwise operations to rotate bits in given number.
 * 
 * @param b Border to rotate (see enum borders).
 * @param places Number of places to rotate.
 * @param direction Direction to rotate (see enum direction_rules).
 * @return Rotated border.
 */
char rotate_border(char b, int places, char direction);

/**
 * @brief Corrects direction of border rotation.
 * @details Determines direction of border rotation by checking direction of triangle field
 * 			on given coordinations.
 * 
 * @param row Row (indexed from 0).
 * @param col Column (indexed from 0).
 * @param defaultdir Default rotation direction of searching algorithm.
 * @return Corrected direction of rotation. (see enum direction_rules).
 */
char correct_direction(int row, int col, char defaultdir);

/**
 * @brief Initializes path search and calls appropriate algorithm function.
 * @details Checks, if file actually exists, if so, loads map from it, checks 
 * 			coordinations' range and calls appropriate function from stucture
 * 			path_arg_t.
 * 
 * @param row Starting row (indexed from 1).
 * @param col Starting row (indexed from 1).
 * @param alg Algorithm string parsed from command line.
 * @param file File with map matrix.
 * @return Error code (see enum ecodes).
 */
int init_search(int row, int col, const char *alg, const char *file);

/**
 * @brief Determies wayback border of given field.
 * @details Wayback border is border which allows return from
 * 			actual field to previous one.
 * 
 * @param inBorder Input border of given field.
 * @return Wayback border.
 */
char wayback_border(char inBorder);

/**
 * @brief Determinates coordinations of next field and saves them to their
 * 		appropriate variables.
 * @details Next coordinations are determined by following border passed by
 * 			variable nextBorder.
 * 
 * @param row Row of current field (indexed from 0).
 * @param col Column of current field (indexed from 0).
 * @param nextBorder Type of following border (see enum borders).
 */
void update_coords(int *row, int *col, char nextBorder);

/**
 * @brief Finds a path in given labyrinth map by following the left-hand rule.
 * 
 * @param map Valid pointer to filled Map structure.
 * @param row Starting row (indexed from 1).
 * @param col Starting column (indexed from 1).
 * @return Error code (see enum ecodes).
 */
int lpath(Map *map, int row, int col);

/**
 * @brief Finds a path in given labyrinth map by following the right-hand rule.
 * 
 * @param map Valid pointer to filled Map structure.
 * @param row Starting row (indexed from 1).
 * @param col Starting column (indexed from 1).
 * @return Error code (see enum ecodes).
 */
int rpath(Map *map, int row, int col);

/**
 * @brief Initializes queue and allocates memory for queue buffer.
 * 
 * @param s Valid pointer to queue_t structure.
 * @param size Size of the queue. In case of invalid size E_IRANGE is returned.
 * 
 * @return E_IRANGE, E_OOM or E_OK (see enum ecodes).
 */
int queue_init(queue_t *q, int size);

/**
 * @brief Deallocates queue's buffer.
 * 
 * @param s Valid pointer to queue_t structure.
 */
void queue_destroy(queue_t *q);

/**
 * @brief Inserts item c at the end of the queue q.
 * 
 * @param q Valid pointer to queue_t structure.
 * @param c Valid pointer to coords_t structure.
 * 
 * @return Always E_OK (so sophisticated!).
 */
int queue_push(queue_t *q, coords_t *c);

/**
 * @brief Removes and saves an item from the front of the queue q into parameter c.
 * 
 * @param q Valid pointer to queue_t structure.
 * @param c Valid pointer to coords_t structure.
 * 
 * @return E_QEMPTY if the queue is empty, E_OK otherwise.
 */
int queue_pop(queue_t *q, coords_t *c);

/**
 * @brief Removes and saves an item from the end of the queue q into parameter c.
 * 
 * @param q Valid pointer to queue_t structure.
 * @param c Valid pointer to coords_t structure.
 * 
 * @return E_QEMPTY if the queue is empty, E_OK otherwise.
 */
int queue_pop_last(queue_t *q, coords_t *c);

/**
 * @brief Checks, if queue q contains c.
 * 
 * @param q Valid pointer to queue_t structure.
 * @param c Valid pointer to coords_t structure.
 * 
 * @return True, if c is in the queue q, false otherwise.
 */
bool queue_check(queue_t *q, coords_t *c);

/**
 * @brief Checks, if given coords are exit coords.
 * 
 * @param map Valid pointer to Map structure.
 * @param row Row (indexed from 0).
 * @param col Column (indexed from 0).
 * @return True, if given coords are exit coords, false otherwise.
 */
bool shortest_iswayout(Map *map, int row, int col);

/**
 * @brief Finds shortest path from given coords in the labyrinth.
 * @details Search is implemented with the use of BFS (Breadth First Search) 
 * 			algorithm with CLOSED queue. (Thanks to Martin Bříza for hint).
 *	 
 * @param map Valid pointer to filled Map structure.
 * @param row Starting row (indexed from 1).
 * @param col Starting column (indexed from 1).
 * @return Error code (see enum ecodes).
 */
int shortest(Map *map, int row, int col);

/** Global variables **/

const struct path_arg_t pa [] = {
	{"--lpath",		lpath},
	{"--rpath",		rpath},
	{"--shortest",	shortest},
	{NULL,		NULL}
};

/** Function definitions **/

int main(int argc, char *argv[])
{
	int ec = E_OK;
	double x, y;
	Map m;

	if(argc == 2 && strcmp(argv[1], "--help") == 0) {
		printf(	"Frantisek Sumsal, xsumsa01@stud.fit.vutbr.cz\n"
				"Project #3: Searching path in labyrinth.\n\n"
				"Usage:\n"
				"\t--help\t\t\tPrints this message\n"
				"\t--test file\t\tTests map in given file. Retruns 'Valid' if given map is valid, 'Invalid' otherwise.\n"
				"\t--rpath R C file\tFinds a path in labyrinth from 'file' using right-hand rule. R and C are coordinations of entry field.\n"
				"\t--lpath R C file\tFinds a path in labyrinth from 'file' using left-hand rule. R and C are coordinations of entry field.\n"
				"\t--shortest R C file\tFinds a shortest path in labyrinth from 'file'. R and C are coordinations of entry field.\n");
	} else if(argc == 3 && strcmp(argv[1], "--test") == 0) {
		if((ec = load_map(argv[2], &m, true)) == E_OK) {
			puts("Valid");
		} else if(ec != E_IFILE) {
			puts("Invalid");
		}
	} else if(argc == 5) {
		if(sscanf(argv[2], "%lf", &x) != 1 || sscanf(argv[3], "%lf", &y) != 1 || (int)x - x != 0 || (int)y - y != 0) {
			fprintf(stderr, "Invalid coordinations: starting coordinations must be integers > 0\n");
			ec = E_IVAL;
		}

		if(ec == E_OK && (ec = init_search((int)x, (int)y, argv[1], argv[4])) == E_IARG) {
			fprintf(stderr, "Invalid arguments, try %s --help\n", argv[0]);
		}
	} else {
		fprintf(stderr, "Invalid arguments, try %s --help\n", argv[0]);
		ec = E_IARG;
	}

	return ec;
}

void* emalloc(size_t size)
{
	void *t = malloc(size);
	if(t == NULL) {
		perror("[ERROR] Could not allocate memory");
	}

	return t;
}

char *ecode_string(int ec)
{
	switch(ec) {
		case E_IRANGE:
			return "Invalid range of arguments.";
		break;
		case E_IARG:
			return "Invalid arguments.";
		break;
		case E_IFILE:
			return "Invalid file.";
		break;
		case E_OOM:
			return "Out of memory.";
		break;
		case E_OTHER:
			return "Other error.";
		break;
		case E_IVAL:
			return "Invalid value.";
		break;
		case E_OK:
			return "";
	}

	return "";
}

int init_map(Map *map, int rows, int cols)
{
	map->rows = rows;
	map->cols = cols;
	map->cells = emalloc(rows * cols * sizeof *map->cells);

	return (map->cells == NULL ? E_OOM : E_OK);
}

void free_map(Map *map)
{
	if(map != NULL) {
		if(map->cells != NULL) {
			free(map->cells);
			map->cells = NULL;
		}
	}
}

int load_map(const char *file, Map *map, bool test)
{
	FILE *in = NULL;
	char *buffer = NULL;
	int rows, cols, n, tmp, ec = E_OK;
	unsigned int offset = 0;
	struct stat stbuf;
	size_t buffsize;

	if((in = fopen(file, "r")) == NULL) {
		fprintf(stderr, "Unable to open file %s\n", file);
		ec = E_IFILE;
	}

	if(ec == E_OK) {
		if((stat(file, &stbuf) != 0 ) || !S_ISREG(stbuf.st_mode)) {
			ec = E_OTHER;
		} else {
			buffsize = stbuf.st_size;
		}

		if(ec == E_OK) {
			/* This will cause bigger memory load - but 
			 * it should be good enough for this project.
			 */
			if((buffer = emalloc(buffsize * sizeof *buffer)) == NULL) {
				ec = E_OOM;
			}

			if(ec == E_OK && fgets(buffer, buffsize, in) != NULL) {
				if(sscanf(buffer, "%d %d", &rows, &cols) != 2) {
					ec = E_IVAL;
				} else {
					if(rows < 1 || cols < 1) {
						ec = E_IRANGE;
					}
				}
			}
		}

		if(ec == E_OK) {
			ec = init_map(map, rows, cols);

			if(ec == E_OK) {
				for(int i = 0; i < rows && ec == E_OK; i++) {
					if(fgets(buffer, buffsize, in) == NULL) {
						ec = E_IVAL;
						break;
					}

					for(int j = 0; j < cols; j++) {
						if(sscanf(&buffer[offset], "%d%n", &n, &tmp) != 1) {
							ec = E_IVAL;
							break;
						}

						offset += tmp;

						if(n < 0 || n > 7) {
							ec = E_IRANGE;
							break;
						}

						map->cells[i * map->cols + j] = n;
					}

					offset = 0;
				}
			}
		}
		
		if(ec == E_OK) {
			ec = check_shared_borders(map);
		}

		if(ec != E_OK || test != false) free_map(map);
		free(buffer);
		fclose(in);
	}

	return ec;
}

int check_shared_borders(Map *map)
{
	int ec = E_OK;
	bool b1, b2;

	for(int i = 0; i < map->rows && ec == E_OK; i++) {
		for(int j = 0; j < map->cols; j++) {
			/* If we're not at last column, check right border
				and left border of next field */
			if(j != map->cols - 1) {
				b1 = isborder(map, i, j, B_RIGHT);
				b2 = isborder(map, i, j + 1, B_LEFT);

				if((b1 || b2) && !(b1 && b2)) {
					ec = E_IVAL;
					break;
				}
			}

			/* If we're not at first column, check left border
				and right border of previous column */
			if(j != 0) {
				b1 = isborder(map, i, j, B_LEFT);
				b2 = isborder(map, i, j -1, B_RIGHT);

				if((b1 || b2) && !(b1 && b2)) {
					ec = E_IVAL;
					break;
				}
			}

			/* If we're not at last row and we're at even row and odd column or odd row and even column,
				check bottom border and top border of field below. */
			if(i != map->rows - 1 && ((i % 2 == 0 && j % 2 != 0) || (i % 2 != 0 && j % 2 == 0))) {
				b1 = isborder(map, i, j, B_TOPDOWN);
				b2 = isborder(map, i + 1, j, B_TOPDOWN);

				if((b1 || b2) && !(b1 && b2)) {
					ec = E_IVAL;
					break;
				}
			}

			/* If we're not at first row and we're at even row and even column or odd row and odd column,
				check top border and bottom border of field above. */
			if(i != 0 && ((i % 2 == 0 && j % 2 == 0) || (i % 2 != 0 && j % 2 != 0))) {
				b1 = isborder(map, i, j, B_TOPDOWN);
				b2 = isborder(map, i - 1, j, B_TOPDOWN);

				if((b1 || b2) && !(b1 && b2)) {
					ec = E_IVAL;
					break;
				}
			}
		}
	}

	return ec;
}

bool isborder(Map *map, int x, int y, int border)
{
	return map->cells[x * map->cols + y] & border; 
}

int start_border(Map *map, int x, int y, int leftright)
{
	/* These rules assume map indexing from 1 */
	if(leftright == D_RIGHT) {
		if((x != 1 && y == 1 && x % 2 != 0) || (x == map->rows)) {
			/* If left odd (but not left top) or bottom */
			return B_RIGHT;
		} else if((x == 1) || (y == map->cols && x % 2 == 0)) {
			/* Top or right with bottom border (even) */
			return B_LEFT;
		} else if((y == 1 && x % 2 == 0) || (y == map->cols && x % 2 != 0)) {
			/* Left even or right with top border (odd) */
			return B_TOPDOWN;
		}
	} else if(leftright == D_LEFT) {
		if((y == 1 && x % 2 == 0) || (x == 1)) {
			/* Left even or top or bottom left */
			return B_RIGHT;
		} else if((x == map->rows) || (y == map->cols && x % 2 != 0)) {
			/* Bottom or right with top border (odd) */
			return B_LEFT;
		} else if((y == 1 && x % 2 != 0) || (y == map->cols && x % 2 == 0)) {
			/* Left odd or right with bottom border (even) */
			return B_TOPDOWN;
		}
	}

	return -1;
}

char rotate_border(char b, int places, char direction)
{
	char n = 0;

	if(direction == D_RIGHT) {
		n = b >> (places % 3);
		n |= (b << (3 - places % 3));
	} else {
		n = b << (places % 3);
		n |= (b >> (3 - places % 3));
	}

	/* Clear unused bits with mask 0000 0111 = 7 */
	return n & 7;
}

char correct_direction(int row, int col, char defaultdir)
{
	char newd = defaultdir;

	if((row % 2 == 0 && col % 2 == 0) || (row % 2 != 0 && col % 2 != 0)) {
		newd = (newd == D_LEFT) ? D_RIGHT : D_LEFT;
	}

	return newd;
}

int init_search(int row, int col, const char *alg, const char *file)
{
	Map m;
	int ec;

	if((ec = load_map(file, &m, false)) != E_OK) {
		fprintf(stderr, "[ERROR] Failed to load map from file %s. Reason: %s\n", file, ecode_string(ec));
	}

	if(ec == E_OK && (row < 1 || row > m.rows || col < 1 || col > m.cols)) {
		fprintf(stderr, "[ERROR] Invalid range of coordinates: [%d, %d]\n", row, col);
		ec = E_IVAL;
	}

	if(ec == E_OK) {
		ec = E_IARG;

		for(unsigned int i = 0; pa[i].name; i++) {
			if(strcmp(alg, pa[i].name) == 0) {
				ec = pa[i].f(&m, row, col);
				break;
			}
		}

		free_map(&m);
	}

	return ec;
}

char wayback_border(char inBorder)
{
	char wBack;

	switch(inBorder) {
		case B_RIGHT:
			wBack = B_LEFT;
		break;
		case B_LEFT:
			wBack = B_RIGHT;
		break;
		case B_TOPDOWN:
			wBack = B_TOPDOWN;
		break;
		/* Whoah, no wayback border! */
		default:
			wBack = -1;
	}

	return wBack;
}

void update_coords(int *row, int *col, char nextBorder) 
{
	switch(nextBorder) {
		case B_RIGHT:
			(*col)++;
		break;
		case B_LEFT:
			(*col)--;
		break;
		case B_TOPDOWN:
			/* These rules assume map indexing from 0 (unlike function start_border()).
			 * Top border:		odd row x odd col || even row x even col
			 * Bottom border:	even row x odd col || odd row x even col
			*/
			if((*row % 2 == 0 && *col % 2 != 0) || (*row % 2 != 0 && *col % 2 == 0)) {
				/* Bottom border */
				(*row)++;
			} else {
				/* Top border */
				(*row)--;
			}
		break;
	}
}

int lpath(Map *map, int row, int col)
{
	int ec = E_OK;
	int nrow = row - 1;
	int ncol = col - 1;
	char wayback;
	char defborder = D_LEFT;
	bool foundwo = false;
	border_t border;

	border.b = start_border(map, row, col, defborder);
	border.d = correct_direction(nrow, ncol, defborder);

	while(foundwo == false) {
		printf("%d,%d\n", nrow + 1, ncol + 1);

		for(short int i = 0; isborder(map, nrow, ncol, border.b) != false && i < 3; i++) {
			border.b = rotate_border(border.b, 1, border.d);
		}

		wayback = wayback_border(border.b);

		if(wayback == -1) {
			printf("Couldn't find a way out from coords [%d, %d]\n", row, col);
			ec = E_OTHER;
			break;
		}

		update_coords(&nrow, &ncol, border.b);
		border.d = correct_direction(nrow, ncol, defborder);
		border.b = rotate_border(wayback, 1, border.d);

		if(nrow < 0 || nrow >= map->rows || ncol < 0 || ncol >= map->cols) {
			foundwo = true;
		}
	}

	return ec;
}

int rpath(Map *map, int row, int col)
{
	int ec = E_OK;
	int nrow = row - 1;
	int ncol = col - 1;
	char wayback;
	char defborder = D_RIGHT;
	bool foundwo = false;
	border_t border;

	border.b = start_border(map, row, col, defborder);
	border.d = correct_direction(nrow, ncol, defborder);

	while(foundwo == false) {
		printf("%d,%d\n", nrow + 1, ncol + 1);

		for(short int i = 0; isborder(map, nrow, ncol, border.b) != false && i < 3; i++) {
			border.b = rotate_border(border.b, 1, border.d);
		}

		wayback = wayback_border(border.b);

		if(wayback == -1) {
			printf("Couldn't find a way out from coords [%d, %d]\n", row, col);
			ec = E_OTHER;
			break;
		}

		update_coords(&nrow, &ncol, border.b);
		border.d = correct_direction(nrow, ncol, defborder);
		border.b = rotate_border(wayback, 1, border.d);

		if(nrow < 0 || nrow >= map->rows || ncol < 0 || ncol >= map->cols) {
			foundwo = true;
		}
	}

	return ec;
}

int queue_init(queue_t *q, int size)
{
	if(size <= 0) {
		return E_IRANGE;
	}

	if((q->buffer = malloc(size * sizeof *(q->buffer))) == NULL) {
		return E_OOM;
	}

	q->size = size;
	q->start = q->end = 0;

	return E_OK;
}

void queue_destroy(queue_t *q)
{
	if(q->buffer) {
		free(q->buffer);
		q->buffer = NULL;
	}
}

int queue_push(queue_t *q, coords_t *c)
{
	memcpy(q->buffer + q->end, c, sizeof *(q->buffer));
	q->end = (q->end + 1) % q->size;
	if(q->end == q->start) {
		q->start = (q->start + 1) % q->size;
	}

	return E_OK;
}

int queue_pop(queue_t *q, coords_t *c)
{
	if(q->end == q->start) {
		return E_QEMPTY;
	}

	memcpy(c, q->buffer + q->start, sizeof *c);
	q->start = (q->start + 1) % q->size;

	return E_OK;
}

int queue_pop_last(queue_t *q, coords_t *c)
{
	if(q->end == q->start) {
		return E_QEMPTY;
	}

	memcpy(c, q->buffer + q->end - 1, sizeof *c);
	q->end = (q->end - 1) % q->size;

	return E_OK;
}

bool queue_check(queue_t *q, coords_t *c)
{
	for(int i = q->start; i != q->end; i = (i + 1) % q->size) {
		if(q->buffer[i].x == c->x && q->buffer[i].y == c->y)
			return true;
	}

	return false;
}

bool shortest_iswayout(Map *map, int row, int col)
{
	bool iswo = false;
	
	if(row == 0 && col % 2 == 0 && isborder(map, row, col, B_TOPDOWN) == false) {
		/* Top and even column */
		iswo = true;
	} else if(col == 0 && isborder(map, row, col, B_LEFT) == false) {
		/* First column */
		iswo = true;
	} else if(row == map->rows - 1 && ((row % 2 != 0 && col % 2 == 0) || (row % 2 == 0 && col % 2 != 0)) && isborder(map, row, col, B_TOPDOWN) == false) {
		/* Bottom and (odd row and even column) or (even row and odd column) */
		iswo = true;
	} else if(col == map->cols - 1 && isborder(map, row, col, B_RIGHT) == false) {
		/* Last column */
		iswo = true;
	}

	return iswo;
}

int shortest(Map *map, int row, int col)
{
	int ec = E_OK;
	bool foundwo = false;
	queue_t open, closed;
	coords_t cnew, cprocess;

	if((ec = queue_init(&open, map->rows * map->cols + 1)) != E_OK) {
		fprintf(stderr, "%s: Unable to initialize queue 'open'. (ec: %d)\n", __FUNCTION__, ec);
	}

	if(ec == E_OK && (ec = queue_init(&closed, map->rows * map->cols + 1)) != E_OK) {
		fprintf(stderr, "%s: Unable to initialize queue 'closed'. (ec: %d)\n", __FUNCTION__, ec);
	}

	if(ec == E_OK) {
		cnew.xp = -1;
		cnew.yp = -1;
		cnew.x = row - 1;
		cnew.y = col - 1;
		queue_push(&open, &cnew);

		while(queue_pop(&open, &cprocess) == E_OK) {
			queue_push(&closed, &cprocess);

			if(shortest_iswayout(map, cprocess.x, cprocess.y) != false) {
				foundwo = true;
				break;
			}

			if(isborder(map, cprocess.x, cprocess.y, B_LEFT) == false && cprocess.y != 0) {
				/* Left expansion */
				cnew.xp = cprocess.x;
				cnew.yp = cprocess.y;
				cnew.x = cprocess.x;
				cnew.y = cprocess.y - 1;
				if(queue_check(&open, &cnew) == false && queue_check(&closed, &cnew) == false) {
					queue_push(&open, &cnew);
				}
			}

			if(isborder(map, cprocess.x, cprocess.y, B_RIGHT) == false && cprocess.y != map->cols - 1) {
				/* Right expansion */
				cnew.xp = cprocess.x;
				cnew.yp = cprocess.y;
				cnew.x = cprocess.x;
				cnew.y = cprocess.y + 1;
				if(queue_check(&open, &cnew) == false && queue_check(&closed, &cnew) == false) {
					queue_push(&open, &cnew);
				}
			}

			if(isborder(map, cprocess.x, cprocess.y, B_TOPDOWN) == false) {
				cnew.xp = cprocess.x;
				cnew.yp = cprocess.y;

				if(cprocess.x != 0 && ((cprocess.x % 2 == 0 && cprocess.y % 2 == 0) || (cprocess.x % 2 != 0 && cprocess.y % 2 != 0))) {
					/* Top expansion */
					cnew.x = cprocess.x - 1;
					cnew.y = cprocess.y;
				} else if(cprocess.x != map->rows - 1 && ((cprocess.x % 2 == 0 && cprocess.y % 2 != 0) || (cprocess.x % 2 != 0 && cprocess.y % 2 == 0))) {
					/* Bottom expansion */
					cnew.x = cprocess.x + 1;
					cnew.y = cprocess.y;
				}

				if(queue_check(&open, &cnew) == false && queue_check(&closed, &cnew) == false) {
					queue_push(&open, &cnew);
				}
			}
		}

		if(foundwo) {
			/* Clear open queue, we'll use it to save back path */
			open.start = open.end = 0;

			queue_pop_last(&closed, &cnew);
			queue_push(&open, &cnew);

			/* Track back path by searching parent coordinations */
			while(queue_pop_last(&closed, &cprocess) == E_OK) {
				if(cnew.xp == -1 && cnew.yp == -1) {
					break;
				}

				if(cnew.xp == cprocess.x && cnew.yp == cprocess.y) {
					cnew = cprocess;
					queue_push(&open, &cnew);
				}
			}

			/* Print back path */
			while(queue_pop_last(&open, &cprocess) == E_OK) {
				printf("%d,%d\n", cprocess.x + 1, cprocess.y + 1);
			}
		} else {
			printf("Couldn't find a way out from coords [%d, %d]\n", row, col);
		}
	}

	queue_destroy(&open);
	queue_destroy(&closed);

	return ec;
}