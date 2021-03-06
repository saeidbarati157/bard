#ifndef _POET_H
#define _POET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef FIXED_POINT
typedef int32_t real_t;
#else
typedef double real_t;
#endif

/**
 * Setting this environment variable tells POET not to execute the
 * poet_apply_control function.
 * Allows disabling POET at runtime, removing the overhead of calculations.
 * Of course, no system changes will then be made either.
 */
#define POET_DISABLE_CONTROL "POET_DISABLE_CONTROL"

/**
 * Setting this environment variable tells POET not to call the apply function.
 * POET will run all its calculations but not make any system changes.
 */
#define POET_DISABLE_APPLY "POET_DISABLE_APPLY"

/**
 * Setting this environment variable tells POET not to use idle states.
 */
#define POET_DISABLE_IDLE "POET_DISABLE_IDLE"

typedef enum {
  PERFORMANCE,
  POWER,
} poet_tradeoff_type_t;

typedef struct poet_internal_state poet_state;

/**
 * The apply function format required to be passed to poet_init().
 */
typedef void (* poet_apply_func) (void * states,
                                  unsigned int num_states,
                                  unsigned int id,
                                  unsigned int last_id,
                                  unsigned long long idle_ns,
                                  unsigned int is_first_apply);

/**
 * The current state function is used to determine the id of the current state
 * of the system before any changes are applied. Should return -1 if the state
 * cannot be determined, 0 otherwise.
 */
typedef int (* poet_curr_state_func) (const void* states,
                                      unsigned int num_states,
                                      unsigned int* curr_state_id);

/**
 * Defines properties of system states.
 * Speedup and cost (e.g. power) are normalized to the lowest state, which
 * should have id=0.
 * Idle_partner_id is used only for idle states. It declares the id of another
 * state with the same exact configuration which does not idle - usually the
 * state with speedup=1 and cost=1 (state 1 when there is a single idle state
 * with id=0).
 */
typedef struct {
  unsigned int id;
  real_t speedup;
  real_t cost;
  unsigned int idle_partner_id;
} poet_control_state_t;

/**
 * Initializes a poet_state struct which is needed to call other functions.
 *
 * A copy of the apply_states struct will be passed to the apply function as
 * the void* parameter when it is called from poet_apply_control(). It is
 * allowed to be NULL, in which case the apply function must know where to
 * access the appropriate data structures to apply system changes.
 *
 * Default values for state variables are located in src/poet_constants.h
 *
 * @param goal
 *   Must be > 0
 * @param constraint
 * @param num_system_states
 *   Must be > 0
 * @param control_states
 *   Must not be NULL
 * @param apply_states
 * @param apply
 * @param current
 * @param period
 *   Must be > 0
 * @param buffer_depth
 *   Must be > 0 if log_filename is specified
 * @param log_filename
 *
 * @return poet_state pointer, or NULL on failure (errno will be set)
 */
poet_state * poet_init(real_t goal,
                       poet_tradeoff_type_t constraint,
                       unsigned int num_system_states,
                       poet_control_state_t * control_states,
                       void * apply_states,
                       poet_apply_func apply,
                       poet_curr_state_func current,
                       unsigned int period,
                       unsigned int buffer_depth,
                       const char * log_filename);

/**
 * Deallocates memory from the poet_state struct.
 *
 * @param state
 */
void poet_destroy(poet_state * state);

/**
 * Change the constraint at runtime.
 *
 * @param state
 * @param constraint
 * @param goal
 */
void poet_set_constraint_type(poet_state * state,
                              poet_tradeoff_type_t constraint,
                              real_t goal);

/**
 * Runs POET decision engine and requests system changes by calling the apply
 * function provided in poet_init().
 *
 * @param state
 * @param id
 *   user-specified identifier for current iteration
 * @param perf
 *   the actual achieved performance
 * @param pwr
 *   the actual achieved power
 */
void poet_apply_control(poet_state * state,
                        unsigned long id,
                        real_t perf,
                        real_t pwr);

#ifdef __cplusplus
}
#endif

#endif
