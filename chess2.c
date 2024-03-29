/***************************************************************************/
/*                               micro-Max,                                */
/* A chess program smaller than 2KB (of non-blank source), by H.G. Muller  */
/* Port to Atmel ATMega644 and AVR GCC, by Andre Adrian                    */
/***************************************************************************/
/* version 4.8 (1953 characters) features:                                 */
/* - recursive negamax search                                              */
/* - all-capture MVV/LVA quiescence search                                 */
/* - (internal) iterative deepening                                        */
/* - best-move-first 'sorting'                                             */
/* - a hash table storing score and best move                              */
/* - futility pruning                                                      */
/* - king safety through magnetic, frozen king in middle-game              */
/* - R=2 null-move pruning                                                 */
/* - keep hash and repetition-draw detection                               */
/* - better defense against passers through gradual promotion              */
/* - extend check evasions in inner nodes                                  */
/* - reduction of all non-Pawn, non-capture moves except hash move (LMR)   */
/* - full FIDE rules (expt under-promotion) and move-legality checking     */

/* 26nov2008 no hash table                                                 */
/* 29nov2008 all IO via myputchar(), mygetchar(), pseudo random generator  */

//#define UNIX
//#define VERBOSE
#ifdef UNIX
#include <stdio.h>
#include <time.h>
#endif

#include "main.h"
#include "vga.h"

//#define W while
#define M 0x88
#define S 128
#define I 8000

long N, T; /* N=evaluated positions+S, T=recursion limit */
short Q, O, K, R, k = 16; /* k=moving side */
char *p, c[5], Z; /* p=pointer to c, c=user input, computer output, Z=recursion counter */

char L, w[] = { 0, 2, 2, 7, -1, 8, 12, 23 }, /* relative piece values    */
o[] = { -16, -15, -17, 0, 1, 16, 0, 1, 16, 15, 17, 0, 14, 18, 31, 33, 0, /* step-vector lists */
7, -1, 11, 6, 8, 3, 6, /* 1st dir. in o[] per piece*/
6, 3, 5, 7, 4, 5, 3, 6 }, /* initial piece setup      */
b[] = { /* board is left part, center-pts table is right part, and dummy */22, 19, 21, 23, 20, 21, 19, 22, 28, 21, 16, 13, 12, 13, 16, 21,
		18, 18, 18, 18, 18, 18, 18, 18, 22, 15, 10, 7, 6, 7, 10, 15, 0, 0, 0, 0, 0, 0, 0, 0, 18, 11, 6, 3, 2, 3, 6, 11, 0, 0, 0, 0, 0, 0, 0,
		0, 16, 9, 4, 1, 0, 1, 4, 9, 0, 0, 0, 0, 0, 0, 0, 0, 16, 9, 4, 1, 0, 1, 4, 9, 0, 0, 0, 0, 0, 0, 0, 0, 18, 11, 6, 3, 2, 3, 6, 11, 9,
		9, 9, 9, 9, 9, 9, 9, 22, 15, 10, 7, 6, 7, 10, 15, 14, 11, 13, 15, 12, 13, 11, 14, 28, 21, 16, 13, 12, 13, 16, 21, 0 };

volatile char breakpoint; /* for debugger */

/* User interface routines */
void myputchar(char c) {
#ifdef UNIX
	putchar(c);
#endif
}

void myputs(const char *s) {
	while (*s)
		myputchar(*s++);
	myputchar('\n');
}

char mygetchar(void) {
#ifdef VERBOSE
	return getchar();
#else
	return 10; /* self play computer with computer */
#endif
}

/* 16bit pseudo random generator */
#define MYRAND_MAX 65535

unsigned short r = 1; /* pseudo random generator seed */

void mysrand(unsigned short r_) {
	r = r_;
}

unsigned short myrand(void) {
	return r = ((r << 11) + (r << 7) + r) >> 1;
}

short D(q, l, e, E, z, n)
	/* recursive minimax search */
	short q, l, e; /* (q,l)=window, e=current eval. score, */
	unsigned char E, z, n; /* E=e.p. sqr.z=prev.dest, n=depth; return score */
{
	short m, v, i, P, V, s;
	unsigned char t, p, u, x, y, X, Y, H, B, j, d, h, F, G, C;
	signed char r;

	if (++Z > 30) { /* stack underrun check */
		breakpoint = 1; /* AVR Studio 4 Breakpoint for stack underrun */
		myputchar('u');
		--Z;
		return e;
	}

	q--; /* adj. window: delay bonus */
	k ^= 24; /* change sides             */
	d = Y = 0; /* start iter. from scratch */
	X = myrand() & ~M; /* start at random field    */
	while (d++ < n || d < 3 || /* iterative deepening loop */
	z & K == I && (N < T & d < 98 || /* root: deepen upto time   */
	(K = X, L = Y & ~M, d = 3))) /* time's up: go do best    */
	{
		x = B = X; /* start scan at prev. best */
		h = Y & S; /* request try noncastl. 1st*/
		P = d < 3 ? I : D(-l, 1 - l, -e, S, 0, d - 3); /* Search null move         */
		m = -P < l | R > 35 ? d > 2 ? -I : e:-P; /* Prune or stand-pat       */
		++N; /* node count (for timing)  */
		do {
			u = b[x]; /* scan board looking for   */
			if (u & k) /*  own piece (inefficient!)*/
			{
				r = p = u & 7; /* p = piece type (set r>0) */
				j = o[p + 16]; /* first step vector f.piece*/
				while (r = p > 2 & r < 0 ? -r : -o[++j]) /* loop over directions o[] */
				{
					A: /* resume normal after best */
					y = x;
					F = G = S; /* (x,y)=move, (F,G)=castl.R*/
					do { /* y traverses ray, or:     */
						H = y = h ? Y ^ h : y + r; /* sneak in prev. best move */
						if (y & M)
							break; /* board edge hit           */
						m = E - S & b[E] && y - E < 2 & E - y < 2 ? I : m; /* bad castling             */
						if (p < 3 & y == E)
							H ^= 16; /* shift capt.sqr. H if e.p.*/
						t = b[H];
						if (t & k | p < 3 & !(y - x & 7) - !t)
							break; /* capt. own, bad pawn mode */
						i = 37 * w[t & 7] + (t & 192); /* value of capt. piece t   */
						m = i < 0 ? I : m; /* K capture                */
						if (m >= l & d > 1)
							goto C;
						/* abort on fail high       */
						v = d - 1 ? e : i - p; /* MVV/LVA scoring          */
						if (d - !t > 1) /* remaining depth          */
						{
							v = p < 6 ? b[x + 8] - b[y + 8] : 0; /* center positional pts.   */
							b[G] = b[H] = b[x] = 0;
							b[y] = u | 32; /* do move, set non-virgin  */
							if (!(G & M))
								b[F] = k + 6, v += 50; /* castling: put R & score  */
							v -= p - 4 | R > 29 ? 0 : 20; /* penalize mid-game K move */
							if (p < 3) /* pawns:                   */
							{
								v -= 9 * ((x - 2 & M || b[x - 2] - u) + /* structure, undefended    */
								(x + 2 & M || b[x + 2] - u) - 1 /*        squares plus bias */
								+ (b[x ^ 16] == k + 36)) /* kling to non-virgin King */
								- (R >> 2); /* end-game Pawn-push bonus */
								V = y + r + 1 & S ? 647 - p : 2 * (u & y + 16 & 32); /* promotion or 6/7th bonus */
								b[y] += V;
								i += V; /* change piece, add score  */
							}
							v += e + i;
							V = m > q ? m : q; /* new eval and alpha       */
							C = d - 1 - (d > 5 & p > 2 & !t & !h);
							C = R > 29 | d < 3 | P - I ? C : d; /* extend 1 ply if in check */
							do
								s = C > 2 | v > V ? -D(-l, -V, -v, /* recursive eval. of reply */
								F, 0, C) :
													v; /* or fail low if futile    */
							while (s > q & ++C < d);
							v = s;
							if (z && K - I && v + I && x == K & y == L) /* move pending & in root:  */
							{
								Q = -e - i;
								O = F; /*   exit if legal & found  */
								R += i >> 7;
								--Z;
								return l; /* captured non-P material  */
							}
							b[G] = k + 6;
							b[F] = b[y] = 0;
							b[x] = u;
							b[H] = t; /* undo move,G can be dummy */
						}
						if (v > m) /* new best, update max,best*/
							m = v, X = x, Y = y | S & F; /* mark double move with S  */
						if (h) {
							h = 0;
							goto A;
						} /* redo after doing old best*/
						if (x + r - y | u & 32 | /* not 1st step,moved before*/
						p > 2 & (p - 4 | j - 7 || /* no P & no lateral K move,*/
						b[G = x + 3 ^ r >> 1 & 7] - k - 6 /* no virgin R in corner G, */
						|| b[G ^ 1] | b[G ^ 2]) /* no 2 empty sq. next to R */
						)
							t += p < 5; /* fake capt. for nonsliding*/
						else
							F = y; /* enable e.p.              */
					} while (!t); /* if not capt. continue ray*/
				}
			}
		} while ((x = x + 9 & ~M) - B); /* next sqr. of board, wrap */
		C: if (m > I - M | m < M - I)
			d = 98; /* mate holds to any depth  */
		m = m + I | P == I ? m : 0; /* best loses K: (stale)mate*/
		if (z && d > 2) {
			*c = 'a' + (X & 7);
			c[1] = '8' - (X >> 4);
			c[2] = 'a' + (Y & 7);
			c[3] = '8' - (Y >> 4 & 7);
			c[4] = 0;
			breakpoint = 2; /* AVR Studio 4 Breakpoint for moves, watch c[] */
#ifdef VERBOSE
			printf("%2d ply, %9d searched, score=%6d by %c%c%c%c\n", d - 1, N - S, m, 'a' + (X & 7), '8' - (X >> 4), 'a' + (Y & 7), '8' - (Y >> 4 & 7)); /* uncomment for Kibitz */
#endif
		}
	} /*    encoded in X S,8 bits */
	k ^= 24; /* change sides back        */
	--Z;
	return m += m < e; /* delayed-loss bonus       */
}

void print_board() {
	short N = -1;
	int row = 0, col = 0;
	while (++N < 121) {
		if (N & 8 && (N += 7)) {
			row++;
			col = 0;
			printf("\n");
		} else {
			char c = "\x00\x00\x09\x0b\x0e\x0c\x0a\x0d\x00\x01\x00\x03\x06\x04\x02\x05"[b[N] & 15];
			vga_chess_board_set(7 - row, col++, c);
		}
	}
//		myputchar(N & 8 && (N += 7) ? 10 : ".?inkbrq?I?NKBRQ"[b[N] & 15]); /* Pawn is i */
}

void chess_init(void) {
#ifdef UNIX
	mysrand(time(NULL)); /* make myrand() calls random */
#endif
	do { /* play loop          */
		print_board();
		while(chess_getmove(c));
		K = I; /* invalid move       */
		if (*c - 10)
			K = *c - 16 * c[1] + 799, L = c[2] - 16 * c[3] + 799; /* parse entered move */
		N = 0;
		T = 0x3F; /* T=Computer Play strength */
		if (D(-I, I, Q, O, 1, 3) <= -I + 1)
			break;
		print_board();
		K = I; /* invalid move       */
		N = 0;
		T = 0x3F; /* T=Computer Play strength */
	} while (D(-I, I, Q, O, 1, 3) > -I + 1); /* think or check & do*/
	print_board();
}
