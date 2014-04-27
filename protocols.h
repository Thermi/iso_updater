/*
 * Copyright (C) 2014 Noel Kuntze <noel@familie-kuntze.de>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* definitions of the length of the names */
#define directorylength 11
#define isolength 30
#define sha1sumslength 14

/*Handles http and https URLs*/
int handleHTTP(struct options options);

/*Handles ftp URLs */
int handleFTP(struct options options);

/*Handles rsync URLs*/
int handleRSYNC(struct options options);