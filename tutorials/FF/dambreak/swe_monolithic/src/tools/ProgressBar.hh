/**
 * @file
 * This file is part of SWE.
 *
 * @author Sebastian Rettenberger (rettenbs AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Sebastian_Rettenberger,_M.Sc.)
 *
 * @section LICENSE
 *
 * SWE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 * A simple progress bar using stdout
 */

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <cassert>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <limits>

#include <unistd.h>
#include <sys/ioctl.h>

namespace tools
{

class ProgressBar
{
private:
	/** Local rank (we only do work on rank 0) */
	int m_rank;

	/** Total amount of work */
	float m_totalWork;

	/** Progress bar initialization time */
	time_t m_startTime;

	/** Terminal size */
	unsigned int m_terminalSize;

	/** Rotating bar char */
	unsigned char m_rotatingBar;

public:
	ProgressBar(float totalWork = 1., int rank = 0)
		: m_rank(rank),
		  m_totalWork(totalWork),
		  m_startTime(time(0)),
		  m_rotatingBar(0)
	{
		if (rank != 0)
			return;

#ifdef TIOCGSIZE
		struct ttysize ts;
		ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
		m_terminalSize = ts.ts_cols;
#elif defined(TIOCGWINSZ)
		struct winsize ts;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
		m_terminalSize = ts.ws_col;
#else
		m_terminalSize = 0;
#endif
		if (m_terminalSize > 300)
			// Probably an error due to MPI
			m_terminalSize = MIN_TERM_SIZE;
	}

	/**
	 * @param done The amount of work already done
	 */
	void update(float done)
	{
		if (m_rank != 0 || m_terminalSize < MIN_TERM_SIZE)
			return;

		unsigned int printed = 2;
		std::cout << '\r';
		printed += printTimeLeft(done);
		std::cout << ' ';
		printed += printPercentage(done);
		std::cout << ' ';
		printProgressBar(done, m_terminalSize-printed-2);
		std::cout << ' ';
		printRotatingBar();
		std::cout << std::flush;
	}

	void clear()
	{
		if (m_rank != 0 || m_terminalSize < MIN_TERM_SIZE)
			return;

		std::cout << '\r';
		for (unsigned int i = 0; i < m_terminalSize; i++)
			std::cout << ' ';
		std::cout << '\r';
	}

private:
	/**
	 * @return Number of characters printed
	 */
	unsigned int printTimeLeft(float done)
	{
		float timeLeft;
		if (done <= 0)
			timeLeft = std::numeric_limits<float>::max();
		else
			timeLeft = (time(0) - m_startTime) * (m_totalWork - done) / done;

		std::cout << "Time left: ";

		if (timeLeft < 1) {
			for (int i = 3; i < TIME_SIZE; i++)
				std::cout << ' ';
			std::cout << "< 1";
		} else {
			int digits = ceil(log(timeLeft)/log(10));
			if (digits > TIME_SIZE) {
				// Maximum number we can show
				for (int i = 0; i < TIME_SIZE; i++)
					std::cout << '9';
			} else {
				streamsize oldPrec = std::cout.precision();
				std::ios::fmtflags oldFlags = std::cout.flags();
				streamsize oldWidth = std::cout.width();

				std::cout.precision(std::max(0, TIME_SIZE-digits-2));
				std::cout.setf(std::ios::fixed);
				std::cout.width(TIME_SIZE);

				std::cout << timeLeft;

				std::cout.precision(oldPrec);
				std::cout.flags(oldFlags);
				std::cout.width(oldWidth);
			}
		}

		std::cout << " sec";

		return 11+TIME_SIZE+4;
	}

	/**
	 * @return Number of characters printed
	 */
	unsigned int printPercentage(float done)
	{
		int per = floor(done/m_totalWork*100);

		std::cout << '(';

		streamsize oldWidth = std::cout.width();

		std::cout.width(3);
		std::cout << per;

		std::cout.width(oldWidth);

		std::cout << "% done)";

		return 1+3+7;
	}

	void printProgressBar(float done, unsigned int size)
	{
		if (size < 3)
			return;

		size -= 2; // leave space for []
		unsigned int per = floor(done/m_totalWork * size);

		std::cout << '[';

		for (unsigned int i = 0; i < per; i++)
			std::cout << '=';

		if (per < size) {
			std::cout << '>';
			per++;
		}

		for (unsigned int i = per; i < size; i++)
			std::cout << ' ';

		std::cout << ']';
	}

	void printRotatingBar()
	{
		static const char* CHARS = "|/-\\";

		std::cout << CHARS[m_rotatingBar];

		m_rotatingBar = (m_rotatingBar + 1) % 4;
	}

	static const unsigned int MIN_TERM_SIZE = 80;
	static const int TIME_SIZE = 8;
};

}

#endif // PROGRESSBAR_H
