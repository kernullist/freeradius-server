/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
#ifndef _FR_LIBRADIUS_H
#define _FR_LIBRADIUS_H
/*
 * $Id$
 *
 * @file include/libradius.h
 * @brief Structures and prototypes for the radius library.
 *
 * @copyright 1999-2014 The FreeRADIUS server project
 */

/*
 *  Compiler hinting macros.  Included here for 3rd party consumers
 *  of libradius.h.
 *
 *  @note Defines RCSIDH.
 */
#include <freeradius-devel/build.h>
RCSIDH(libradius_h, "$Id$")

/*
 *  Let any external program building against the library know what
 *  features the library was built with.
 */
#include <freeradius-devel/features.h>

/*
 *  Talloc'd memory must be used throughout the librarys and server.
 *  This allows us to track allocations in the NULL context and makes
 *  root causing memory leaks easier.
 */
#include <talloc.h>

/*
 *  Defines signatures for any missing functions.
 */
#include <freeradius-devel/missing.h>

/*
 *  Include system headers.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <signal.h>

#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif

#include <freeradius-devel/threads.h>
#include <freeradius-devel/inet.h>
#include <freeradius-devel/dict.h>
#include <freeradius-devel/token.h>
#include <freeradius-devel/pair.h>
#include <freeradius-devel/pair_cursor.h>

#include <freeradius-devel/packet.h>
#include <freeradius-devel/radius.h>
#include <freeradius-devel/radius/radius.h>
#include <freeradius-devel/talloc.h>
#include <freeradius-devel/hash.h>
#include <freeradius-devel/regex.h>
#include <freeradius-devel/proto.h>
#include <freeradius-devel/conf.h>
#include <freeradius-devel/radpaths.h>
#include <freeradius-devel/rbtree.h>
#include <freeradius-devel/fr_log.h>
#include <freeradius-devel/version.h>
#include <freeradius-devel/value.h>

#ifdef SIZEOF_UNSIGNED_INT
#  if SIZEOF_UNSIGNED_INT != 4
#    error FATAL: sizeof(unsigned int) != 4
#  endif
#endif

/*
 *  Include for modules.
 */
#include <freeradius-devel/sha1.h>
#include <freeradius-devel/md4.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_SIG_T
typedef void (*sig_t)(int);
#endif

#ifndef NDEBUG
#  define FREE_MAGIC (0xF4EEF4EE)
#endif

/*
 *	Printing functions.
 */
size_t		fr_utf8_char(uint8_t const *str, ssize_t inlen);
ssize_t		fr_utf8_str(uint8_t const *str, ssize_t inlen);
char const     	*fr_utf8_strchr(int *chr_len, char const *str, char const *chr);
size_t		fr_snprint(char *out, size_t outlen, char const *in, ssize_t inlen, char quote);
size_t		fr_snprint_len(char const *in, ssize_t inlen, char quote);
char		*fr_asprint(TALLOC_CTX *ctx, char const *in, ssize_t inlen, char quote);
char		*fr_vasprintf(TALLOC_CTX *ctx, char const *fmt, va_list ap);

#define		is_truncated(_ret, _max) ((_ret) >= (size_t)(_max))
#define		truncate_len(_ret, _max) (((_ret) >= (size_t)(_max)) ? (((size_t)(_max)) - 1) : _ret)

/** Boilerplate for checking truncation
 *
 * If truncation has occurred, advance _p as far as possible without
 * overrunning the output buffer, and \0 terminate.  Then return the length
 * of the buffer we would have needed to write the full value.
 *
 * If truncation has not occurred, advance _p by whatever the copy or print
 * function returned.
 */
#define RETURN_IF_TRUNCATED(_p, _ret, _max) \
do { \
	if (is_truncated(_ret, _max)) { \
		size_t _r = (_p - out) + _ret; \
		_p += truncate_len(_ret, _max); \
		*_p = '\0'; \
		return _r; \
	} \
	_p += _ret; \
} while (0)

extern uint32_t	fr_max_attributes; /* per incoming packet */
#define	FR_MAX_PACKET_CODE (52)
extern char const *fr_packet_codes[FR_MAX_PACKET_CODE];
#define is_radius_code(_x) ((_x > 0) && (_x < FR_MAX_PACKET_CODE))

/*
 *	Several handy miscellaneous functions.
 */
int		fr_set_signal(int sig, sig_t func);
int		fr_talloc_link_ctx(TALLOC_CTX *parent, TALLOC_CTX *child);
int		fr_unset_signal(int sig);
int		rad_lockfd(int fd, int lock_len);
int		rad_lockfd_nonblock(int fd, int lock_len);
int		rad_unlockfd(int fd, int lock_len);
char		*fr_abin2hex(TALLOC_CTX *ctx, uint8_t const *bin, size_t inlen);
size_t		fr_bin2hex(char *hex, uint8_t const *bin, size_t inlen);
size_t		fr_hex2bin(uint8_t *bin, size_t outlen, char const *hex, size_t inlen);
uint64_t	fr_strtoull(char const *value, char **end);
int64_t		fr_strtoll(char const *value, char **end);
bool		is_whitespace(char const *value);
bool		is_printable(void const *value, size_t len);
bool		is_integer(char const *value);
bool		is_zero(char const *value);

int		fr_nonblock(int fd);
int		fr_blocking(int fd);
ssize_t		fr_writev(int fd, struct iovec[], int iovcnt, struct timeval *timeout);

ssize_t		fr_utf8_to_ucs2(uint8_t *out, size_t outlen, char const *in, size_t inlen);
size_t		fr_snprint_uint128(char *out, size_t outlen, uint128_t const num);
int		fr_time_from_str(time_t *date, char const *date_str);
void		fr_timeval_from_ms(struct timeval *out, uint64_t ms);
void		fr_timeval_from_usec(struct timeval *out, uint64_t usec);
void		fr_timeval_subtract(struct timeval *out, struct timeval const *end, struct timeval const *start);
void		fr_timeval_add(struct timeval *out, struct timeval const *a, struct timeval const *b);
void		fr_timeval_divide(struct timeval *out, struct timeval const *in, int divisor);

int		fr_timeval_cmp(struct timeval const *a, struct timeval const *b);
int		fr_timeval_from_str(struct timeval *out, char const *in);
bool		fr_timeval_isset(struct timeval const *tv);

void		fr_timespec_subtract(struct timespec *out, struct timespec const *end, struct timespec const *start);

bool		fr_multiply(uint64_t *result, uint64_t lhs, uint64_t rhs);
int		fr_size_from_str(size_t *out, char const *str);
int8_t		fr_pointer_cmp(void const *a, void const *b);
void		fr_quick_sort(void const *to_sort[], int min_idx, int max_idx, fr_cmp_t cmp);
int		fr_digest_cmp(uint8_t const *a, uint8_t const *b, size_t length) CC_HINT(nonnull);

/*
 *	Define TALLOC_DEBUG to check overflows with talloc.
 *	we can't use valgrind, because the memory used by
 *	talloc is valid memory... just not for us.
 */
#ifdef TALLOC_DEBUG
void		fr_talloc_verify_cb(const void *ptr, int depth,
				    int max_depth, int is_ref,
				    void *private_data);
#define VERIFY_ALL_TALLOC talloc_report_depth_cb(NULL, 0, -1, fr_talloc_verify_cb, NULL)
#else
#define VERIFY_ALL_TALLOC
#endif

#ifdef WITH_ASCEND_BINARY
/* filters.c */
int		ascend_parse_filter(fr_value_box_t *out, char const *value, size_t len);
void		print_abinary(char *out, size_t outlen, uint8_t const *data, size_t len, int8_t quote);
#endif /*WITH_ASCEND_BINARY*/

/* random numbers in isaac.c */
/* context of random number generator */
typedef struct fr_randctx {
	uint32_t randcnt;
	uint32_t randrsl[256];
	uint32_t randmem[256];
	uint32_t randa;
	uint32_t randb;
	uint32_t randc;
} fr_randctx;

void		fr_isaac(fr_randctx *ctx);
void		fr_randinit(fr_randctx *ctx, int flag);
uint32_t	fr_rand(void);	/* like rand(), but better. */
void		fr_rand_seed(void const *, size_t ); /* seed the random pool */


/* crypt wrapper from crypt.c */
int		fr_crypt_check(char const *password, char const *reference_crypt);

/* cbuff.c */

typedef struct fr_cbuff fr_cbuff_t;

fr_cbuff_t	*fr_cbuff_alloc(TALLOC_CTX *ctx, uint32_t size, bool lock);
void		fr_cbuff_rp_insert(fr_cbuff_t *cbuff, void *obj);
void		*fr_cbuff_rp_next(fr_cbuff_t *cbuff, TALLOC_CTX *ctx);

/* debug.c */
typedef enum {
	DEBUGGER_STATE_UNKNOWN_NO_PTRACE		= -3,	//!< We don't have ptrace so can't check.
	DEBUGGER_STATE_UNKNOWN_NO_PTRACE_CAP	= -2,	//!< CAP_SYS_PTRACE not set for the process.
	DEBUGGER_STATE_UNKNOWN			= -1,	//!< Unknown, likely fr_get_debug_state() not called yet.
	DEBUGGER_STATE_NOT_ATTACHED		= 0,	//!< We can attach, so a debugger must not be.
	DEBUGGER_STATE_ATTACHED			= 1	//!< We can't attach, it's likely a debugger is already tracing.
} fr_debug_state_t;

#define FR_FAULT_LOG(fmt, ...) fr_fault_log(fmt "\n", ## __VA_ARGS__)
typedef void (*fr_fault_log_t)(char const *msg, ...) CC_HINT(format (printf, 1, 2));
extern fr_debug_state_t fr_debug_state;

/** Optional callback passed to fr_fault_setup
 *
 * Allows optional logic to be run before calling the main fault handler.
 *
 * If the callback returns < 0, the main fault handler will not be called.
 *
 * @param signum signal raised.
 * @return
 *	- 0 on success.
 *	- < 0 on failure.
 */
typedef int (*fr_fault_cb_t)(int signum);
typedef struct fr_bt_marker fr_bt_marker_t;

void		fr_debug_state_store(void);
char const	*fr_debug_state_to_msg(fr_debug_state_t state);
void		fr_debug_break(bool always);
void		backtrace_print(fr_cbuff_t *cbuff, void *obj);
int		fr_backtrace_do(fr_bt_marker_t *marker);
fr_bt_marker_t	*fr_backtrace_attach(fr_cbuff_t **cbuff, TALLOC_CTX *obj);

void		fr_panic_on_free(TALLOC_CTX *ctx);
int		fr_set_dumpable_init(void);
int		fr_set_dumpable(bool allow_core_dumps);
int		fr_reset_dumpable(void);
int		fr_log_talloc_report(TALLOC_CTX *ctx);
void		fr_fault(int sig);
void		fr_talloc_fault_setup(void);
int		fr_fault_setup(char const *cmd, char const *program);
void		fr_fault_set_cb(fr_fault_cb_t func);
void		fr_fault_set_log_fd(int fd);
void		fr_fault_log(char const *msg, ...) CC_HINT(format (printf, 1, 2));

#  ifdef WITH_VERIFY_PTR
void		fr_pair_verify(char const *file, int line, VALUE_PAIR const *vp);
void		fr_pair_list_verify(char const *file, int line, TALLOC_CTX *expected, VALUE_PAIR *vps);
#  endif

bool		fr_cond_assert_fail(char const *file, int line, char const *expr);

/** Calls panic_action ifndef NDEBUG, else logs error and evaluates to value of _x
 *
 * Should be wrapped in a condition, and if false, should cause function to return
 * an error code.  This allows control to return to the caller if a precondition is
 * not satisfied and we're not debugging.
 *
 * Example:
 @verbatim
   if (!fr_cond_assert(request)) return -1
 @endverbatim
 *
 * @param _x expression to test (should evaluate to true)
 */
#define		fr_cond_assert(_x) (bool)((_x) ? true : (fr_cond_assert_fail(__FILE__,  __LINE__, #_x) && false))

void		NEVER_RETURNS _fr_exit(char const *file, int line, int status);
#  define	fr_exit(_x) _fr_exit(__FILE__,  __LINE__, (_x))

void		NEVER_RETURNS _fr_exit_now(char const *file, int line, int status);
#  define	fr_exit_now(_x) _fr_exit_now(__FILE__,  __LINE__, (_x))

/*
 *	FIFOs
 */
typedef struct	fr_fifo_t fr_fifo_t;
typedef void (*fr_fifo_free_t)(void *);
fr_fifo_t	*fr_fifo_create(TALLOC_CTX *ctx, int max_entries, fr_fifo_free_t freeNode);
int		fr_fifo_push(fr_fifo_t *fi, void *data);
void		*fr_fifo_pop(fr_fifo_t *fi);
void		*fr_fifo_peek(fr_fifo_t *fi);
unsigned int	fr_fifo_num_elements(fr_fifo_t *fi);

/*
 *	socket.c
 */
int		fr_socket(fr_ipaddr_t const *ipaddr, uint16_t port);
int		fr_socket_client_unix(char const *path, bool async);
int		fr_socket_client_udp(fr_ipaddr_t const *src_ipaddr, fr_ipaddr_t const *dst_ipaddr,
				     uint16_t dst_port, bool async);
int		fr_socket_client_tcp(fr_ipaddr_t const *src_ipaddr, fr_ipaddr_t const *dst_ipaddr,
				     uint16_t dst_port, bool async);
int		fr_socket_wait_for_connect(int sockfd, struct timeval const *timeout);
int		fr_socket_server_base(int proto, fr_ipaddr_t *ipaddr, int *port, char const *port_name, bool async);
int		fr_socket_server_bind(int sockfd, fr_ipaddr_t *ipaddr, int *port, char const *interface);
int		fr_is_inaddr_any(fr_ipaddr_t *ipaddr);

#ifdef __cplusplus
}
#endif


#ifdef WITH_TCP
#  include <freeradius-devel/tcp.h>
#endif

#endif /* _FR_LIBRADIUS_H */
