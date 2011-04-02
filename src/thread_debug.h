#ifndef _THREAD_DEBUG_H
#define _THREAD_DEBUG_H

#if DEBUG

# define DTRY(lock) g_debug("TRY: %p (%s) tries to lock " #lock, (gpointer) g_thread_self(), __func__)
# define DLOCKED(lock) g_debug("LOCKED: %p (%s) locked " #lock, (gpointer) g_thread_self(), __func__)
# define DUNLOCKED(lock) g_debug("UNLOCKED: %p (%s) unlocked " #lock, (gpointer) g_thread_self(), __func__)

#else

# define DTRY(lock)
# define DLOCKED(lock)
# define DUNLOCKED(lock)

#endif

#endif
