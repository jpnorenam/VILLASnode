/** Unit tests for memory management
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2020, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#include <criterion/criterion.h>

#include <villas/signal.h>

extern void init_memory();

// cppcheck-suppress unknownMacro
Test(signal_data, parse, .init = init_memory) {
	int ret;
	enum SignalType type;
	union signal_data sd;
	const char *str;
	char *end;

	str = "1";
	type = SignalType::INTEGER;

	ret = signal_data_parse_str(&sd, type, str, &end);
	cr_assert_eq(ret, 0);
	cr_assert_eq(end, str + strlen(str));
	cr_assert_eq(sd.i, 1);

	str = "1.2";
	type = SignalType::FLOAT;

	ret = signal_data_parse_str(&sd, type, str, &end);
	cr_assert_eq(ret, 0);
	cr_assert_eq(end, str + strlen(str));
	cr_assert_float_eq(sd.f, 1.2, 1e-6);

	str = "1";
	type = SignalType::BOOLEAN;

	ret = signal_data_parse_str(&sd, type, str, &end);
	cr_assert_eq(ret, 0);
	cr_assert_eq(end, str + strlen(str));
	cr_assert_eq(sd.b, 1);

	str = "1";
	type = SignalType::COMPLEX;

	ret = signal_data_parse_str(&sd, type, str, &end);
	cr_assert_eq(ret, 0);
	cr_assert_eq(end, str + strlen(str));
	cr_assert_float_eq(std::real(sd.z), 1, 1e-6);
	cr_assert_float_eq(std::imag(sd.z), 0, 1e-6);

	str = "-1-3i";
	type = SignalType::COMPLEX;

	ret = signal_data_parse_str(&sd, type, str, &end);
	cr_assert_eq(ret, 0);
	cr_assert_eq(end, str + strlen(str));
	cr_assert_float_eq(std::real(sd.z), -1, 1e-6);
	cr_assert_float_eq(std::imag(sd.z), -3, 1e-6);

	str = "-3i";
	type = SignalType::COMPLEX;

	ret = signal_data_parse_str(&sd, type, str, &end);
	cr_assert_eq(ret, 0);
	cr_assert_eq(end, str + strlen(str));
	cr_assert_float_eq(std::real(sd.z), 0, 1e-6);
	cr_assert_float_eq(std::imag(sd.z), -3, 1e-6);
}
