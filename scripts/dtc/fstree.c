/*
 * (C) Copyright David Gibson <dwg@au1.ibm.com>, IBM Corporation.  2005.
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *                                                                   USA
 */

#include "dtc.h"

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static struct node *read_fstree(const char *dirname)
{
	DIR *d;
	struct dirent *de;
	struct stat st;
	struct node *tree;

	d = opendir(dirname);
	if (!d)
		die("Couldn't opendir() \"%s\": %s\n", dirname, strerror(errno));

	tree = build_node(NULL, NULL);

	while ((de = readdir(d)) != NULL) {
		char *tmpname;

		if (streq(de->d_name, ".")
		    || streq(de->d_name, ".."))
			continue;

		tmpname = join_path(dirname, de->d_name);

		if (lstat(tmpname, &st) < 0)
			die("stat(%s): %s\n", tmpname, strerror(errno));

		if (S_ISREG(st.st_mode)) {
			struct property *prop;
			FILE *pfile;
			int fd;
			struct stat fst;

			fd = open(tmpname, O_RDONLY | O_NOFOLLOW);
			if (fd < 0) {
				fprintf(stderr,
					"WARNING: Cannot open %s: %s\n",
					tmpname, strerror(errno));
			} else if (fstat(fd, &fst) < 0 || !S_ISREG(fst.st_mode)) {
				fprintf(stderr,
					"WARNING: Cannot stat %s: %s\n",
					tmpname, strerror(errno));
				close(fd);
			} else {
				pfile = fdopen(fd, "rb");
				if (!pfile) {
					fprintf(stderr,
						"WARNING: Cannot open stream %s: %s\n",
						tmpname, strerror(errno));
					close(fd);
				} else {
					prop = build_property(xstrdup(de->d_name),
							      data_copy_file(pfile,
									     fst.st_size));
					add_property(tree, prop);
					fclose(pfile);
				}
			}
		} else if (S_ISDIR(st.st_mode)) {
			struct node *newchild;

			newchild = read_fstree(tmpname);
			newchild = name_node(newchild, xstrdup(de->d_name));
			add_child(tree, newchild);
		}

		free(tmpname);
	}

	closedir(d);
	return tree;
}

struct dt_info *dt_from_fs(const char *dirname)
{
	struct node *tree;

	tree = read_fstree(dirname);
	tree = name_node(tree, "");

	return build_dt_info(DTSF_V1, NULL, tree, guess_boot_cpuid(tree));
}
