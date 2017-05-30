/*
 * Copyright 2017 Alex Zhang aka. ztc1997
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

const int FIRST_INFO_CODE = 128;
const int FIRST_ERROR_CODE = FIRST_INFO_CODE*2;

const int ERROR_MISSING_PARAMETERS = FIRST_ERROR_CODE;
const int ERROR_SERVICE_MANAGER = FIRST_ERROR_CODE + 1;
const int ERROR_SERVICE_BINDER = FIRST_ERROR_CODE + 2;
const int ERROR_INVALID_INPUT = FIRST_ERROR_CODE + 3;
const int ERROR_DECODE_PARCEL = FIRST_ERROR_CODE + 4;
const int ERROR_ENCODE_PARCEL = FIRST_ERROR_CODE + 5;
const int ERROR_CALL_FAILED = FIRST_ERROR_CODE + 6;

// const int INFO_DAEMON_MODE = FIRST_INFO_CODE;
// const int INFO_DAEMON_STOPPED = FIRST_INFO_CODE + 1;
// const int INFO_DAEMON_RUNNING = FIRST_INFO_CODE + 2;
