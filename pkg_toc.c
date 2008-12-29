/*
 * Messy, ugly hack to handle Xar XML TOC (Table Of Contents)
 *
 * This will eventually get rewritten. Feel free to do it yourself
 * and send patches to <bk@nn2.us>
 */
#include <limits.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <zlib.h>
#include "conf.h"
#include "db_sqlite.h"
#include "logger.h"
#include "memory.h"
#include "pkg.h"
#include "str.h"
#include "vfs.h"
#include "xar/xar.h"

size_t	pkg_toc_zbufsize = 0;

#define	FAIL	if (bufcomp) { mem_free(bufcomp); } \
                if (bufuncomp) { mem_free(bufuncomp); } \
                if (tmp) { mem_free(tmp); }\
                return NULL;
 
char *pkg_toc_extract(const char *path) {
        unsigned char *bufcomp = NULL, *bufuncomp = NULL;
        int len;
        int infd, outfd;
        int ret;
        ssize_t	sread;
        struct xar_header h;
        z_stream zs;
        char *tmp = mem_alloc(PATH_MAX);

        if ((infd = open(path, O_RDONLY)) < 0) {
           Log(LOG_ERROR, "error importing pkg %s: %d:%s", path, errno, strerror(errno));
           FAIL
        }

        snprintf(tmp, PATH_MAX - 1, "%s.XXXXXX", path);

        if ((outfd = mkstemp(tmp)) < 0) {
           Log(LOG_ERROR, "error opening toc output file %s: %d:%s", path, errno, strerror(errno));
           FAIL
        }

        if (read(infd, &h, 8) < 0) {
           Log(LOG_ERROR, "error reading Xar header on pkg %s: %d:%s", path, errno, strerror(errno));
           FAIL
        }

        h.magic = ntohl(h.magic);
        h.size = ntohs(h.size);
        h.version = ntohs(h.version);

        len = sizeof(h.magic) + sizeof(h.size) + sizeof(h.version);

        if (read(infd, &h.toc_length_compressed, h.size - len) < 0) {
           Log(LOG_ERROR, "error reading Xar toc size on pkg %s: %d:%s", path, errno, strerror(errno));
           FAIL
        }

        /* BEGIN: Voodoo lifted from toc_extract.c in xar distribution */
        h.toc_length_compressed = xar_ntoh64(h.toc_length_compressed);
        h.toc_length_uncompressed = xar_ntoh64(h.toc_length_uncompressed);
        h.cksum_alg = ntohl(h.cksum_alg);

        bufcomp = mem_alloc(pkg_toc_zbufsize);
        bufuncomp = mem_alloc(pkg_toc_zbufsize);
        zs.zalloc = Z_NULL;
        zs.zfree = Z_NULL;
        zs.opaque = Z_NULL;

        if ((ret = inflateInit(&zs)) != Z_OK) {
           Log(LOG_ERROR, "inflateInit doesnt like us... ret=%d", ret);
           FAIL
        }

        sread = 0;
        while (sread < h.toc_length_compressed) {
           int sz = pkg_toc_zbufsize;

           if (h.toc_length_compressed - sread < pkg_toc_zbufsize)
              sz = h.toc_length_compressed - sread;

           if ((ret = read(infd, bufcomp, sz)) < 0) {
              Log(LOG_ERROR, "error reading Xar toc on pkg %s: %d:%s", path, errno, strerror(errno));
              FAIL
           }

           sread += ret;
           zs.next_in = bufcomp;
           zs.avail_in = ret;

           do {
              int err;
              zs.avail_out = pkg_toc_zbufsize;
              zs.next_out = bufuncomp;

              if ((err = inflate(&zs, Z_SYNC_FLUSH)) != Z_OK) {
                 switch (err) {
                    case Z_NEED_DICT:
                    case Z_DATA_ERROR:
                       Log(LOG_ERROR, "invalid compressed data in toc for %s", path);
                       inflateEnd(&zs);
                       FAIL
                    case Z_MEM_ERROR:
                       Log(LOG_ERROR, "out of memory uncompressing TOC for %s", path);
                       inflateEnd(&zs);
                       FAIL
                 }
              }

              len = pkg_toc_zbufsize - zs.avail_out;
              write(outfd, bufuncomp, len);
           }  while (zs.avail_out == 0);
        }
        /* END: Voodoo lifted from toc_extract.c in xar distribution */

        /* cleanup */
        inflateEnd(&zs);
        close(infd);
        close(outfd);

        return tmp;
}
#undef	FAIL

static char pathbuf[PATH_MAX];

static void pkg_toc_dump(xmlDoc *doc, xmlNode *root, char *path) {
    xmlNode *curr = NULL;
    xmlChar *p = NULL;

    for (curr = root; curr; curr = curr->next) {
          printf("=> %s\n", curr->name);

          if (xmlStrcmp(curr->name, (const xmlChar *)"file")) {
             p = xmlGetProp(curr, (const xmlChar *)"id");
             printf("* file: %s\n", p);
             xmlFree(p);
          } else if (xmlStrcmp(curr->name, (const xmlChar *)"name")) {
             p = xmlNodeListGetString(doc, curr->xmlChildrenNode, 1);
             printf("p: %s", (const char *)p);
/*             strcat(pathbuf, (const char *)p); */
          }
       pkg_toc_dump(doc, curr->children, path);
    }
}

static int32_t pkg_xar_err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx) {
	xar_file_t f;
	xar_t x;
	const char *str;
	int e;

	x = xar_err_get_archive(ctx);
	f = xar_err_get_file(ctx);
	str = xar_err_get_string(ctx);
	e = xar_err_get_errno(ctx);

	switch(sev) {
	case XAR_SEVERITY_DEBUG:
	case XAR_SEVERITY_INFO:
		break;
	case XAR_SEVERITY_WARNING:
		printf("%s\n", str);
		break;
	case XAR_SEVERITY_NORMAL:
		break;
	case XAR_SEVERITY_NONFATAL:
	case XAR_SEVERITY_FATAL:
		if (f) {
		   const char *file = xar_get_path(f);
		   if (file)
		      printf(":(%s)", file);

		   free((char *)file);
		}

		if (str)
		   printf(": %s", str);

		if (err)
		   printf(" (%s)", strerror(e));
		if (sev == XAR_SEVERITY_NONFATAL) {
                   printf(" - ignored");
		   printf("\n");
		} else {
		   printf("\n");
		   exit(1);
		}

		break;
	}
	return 0;
}

#define	TOCLINELEN	384
int pkg_toc_process(const char *path, const char *toc, u_int32_t pkgid) {
#if	0	/* XXX: finish this! */
    char buf[384];	/* a line really shouldnt be longer than this.. */
    char pathbuf[PATH_MAX];
    int	done = 0;
    int line = 0;
    xmlDoc *xml_doc = NULL;
    xmlNode *xml_root = NULL;

    LIBXML_TEST_VERSION

    if ((xml_doc = xmlReadFile(toc, NULL, 0)) == NULL) {
       Log(LOG_ERROR, "error opening toc %s for pkg %s", toc, path);
       return EXIT_FAILURE;
    }

    xml_root = xmlDocGetRootElement(xml_doc);

    pkg_toc_dump(xml_doc, xml_root, pathbuf);
    xmlFreeDoc(xml_doc);
    xmlCleanupParser();
#else	/* XXX: This is vile, ugly, disgusting vomit code. Fix it! -jld */
	xar_t x;
	xar_iter_t i;
	xar_file_t f;
        struct vfs_fake_stat sb;

	if (!(x = xar_open(path, READ))) {
	   Log(LOG_ERROR, "failed opening package %s", path);
	   return EXIT_FAILURE;
	}

	xar_register_errhandler(x, pkg_xar_err_callback, NULL);

        if (!(i = xar_iter_new())) {
           Log(LOG_ERROR, "failed getting new xar iter in pkg %s", path);
           return EXIT_FAILURE;
        }

        for (f = xar_file_first(x, i); f; f = xar_file_next(i)) {
                char *size = xar_get_size(x, f);
                char *xpath = xar_get_path(f);
                char *xtype = xar_get_type(x, f);
                char *mode = xar_get_mode(x, f);
                char *user = xar_get_owner(x, f);
                char *group = xar_get_group(x, f);
                char *mtime = xar_get_mtime(x, f);
                const char *offset;
                char type = 'u';
/*    *	                  (f)ile, (d)irectory, (l)ink, (p)ipe
      *                    f(i)fo, (c)haracter, (b)lock, (s)ocket,
      *                   (u)ndefined
 */
                if (!strcasecmp(xtype, "file")) {
                  xar_prop_get(f, "data/offset", &offset);
                  type = 'f';
                } else {
                      offset = "0";
                      if (!strcasecmp(xtype, "directory"))
                         type = 'd';
                      else if (!strcasecmp(xtype, "hardlink"))
                         type = 'l';
                      else if (!strcasecmp(xtype, "symlink"))
                         type = 'l';
                }

                printf("%s: %s %8s/%-8s %10s %s %s @ %s\n", xtype, mode, user, group, size, mtime, xpath, offset);
                db_file_remove(pkgid, path);
                db_query(QUERY_INT, "INSERT INTO files (package, path, type, owner, "
                                    "group, offset, ctime, mode, target) VALUES (%s, '%s', '%s', %s, "
                                    "%s, %s, %s, %s, '%s', %s);", pkgid, xpath, type, user, group,
                                    size, offset, mtime);

                free(mtime);
                free(group);
                free(user);
                free(mode);
                free(xtype);
                free(xpath);
                free(size);
       }
#endif
    return EXIT_SUCCESS;
}

#undef TOCLINELEN