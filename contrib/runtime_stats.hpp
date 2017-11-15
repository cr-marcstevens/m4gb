/*
* Author:  David Robert Nadeau
* Site:    http://NadeauSoftware.com/
* License: Creative Commons Attribution 3.0 Unported License
*          http://creativecommons.org/licenses/by/3.0/deed.en_US
*/

#ifndef NADEAU_RUNTIME_STATS_HPP
#define NADEAU_RUNTIME_STATS_HPP

#include <cstdint>
#include <cstddef>

/**
* Returns the peak (maximum so far) resident set size (physical
* memory use) measured in bytes, or zero if the value cannot be
* determined on this OS.
*/
std::size_t getPeakRSS();

/**
* Returns the current resident set size (physical memory use) measured
* in bytes, or zero if the value cannot be determined on this OS.
*/
std::size_t getCurrentRSS();

/**
* Returns the amount of CPU time used by the current process,
* in seconds, or -1.0 if an error occurred.
*/
double getCPUTime();

#endif // NADEAU_RUNTIME_STATS_HPP
