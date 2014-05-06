/*
 * config.h:
 *  Configure the behaviour of luawrap with macros.
 */

#ifndef LUAWRAP_CONFIG_H_
#define LUAWRAP_CONFIG_H_

// Allow luawrap::call to take > 6 arguments
//#define LUAWRAP_LONG_CALLS

// Allow luawrap::function to wrap functions with > 6 arguments
//#define LUAWRAP_LONG_FUNCTIONS

// Do not special-case and wrap vector as a lua table
//#define LUAWRAP_NO_WRAP_VECTOR

#define LUAWRAP_USE_EXCEPTIONS

#endif /* LUAWRAP_CONFIG_H_ */
