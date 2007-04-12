/*
 * load.h - Public API for loading files
 *
 *   Copyright (c) 2000-2007 Shiro Kawai <shiro@acm.org>
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  $Id: load.h,v 1.1 2007-04-12 03:26:56 shirok Exp $
 */

/* This file is included from gauche.h */

#ifndef GAUCHE_LOAD_H
#define GAUCHE_LOAD_H

/*=================================================================
 * Loading Scheme files
 */

/* Flags for Scm_VMLoad, Scm_Load, amd Scm_Requre. (not for Scm_VMLoadPort) */
typedef enum {
    SCM_LOAD_QUIET_NOFILE = (1L<<0),  /* do not signal an error if the file
                                         does not exist; just return #f. */
    SCM_LOAD_IGNORE_CODING = (1L<<1), /* do not use coding-aware port to honor
                                         'coding' magic comment */
    SCM_LOAD_PROPAGATE_ERROR = (1L<<2) /* do not capture an error; let the
                                         caller handle it.  Not effective
                                         for Scm_VMLoad. */
} ScmLoadFlags;

/* A structure to obtain a detailed result of loading. */
typedef struct ScmLoadPacketRec {
    ScmObj exception; /* OUT: exception object in case of LOAD_EVAL_ERROR */
    int    loaded;    /* OUT: TRUE iff file is successfully loaded.  */
    ScmObj paths;     /* IN:  0 */
} ScmLoadPacket;

SCM_EXTERN ScmObj Scm_VMLoadFromPort(ScmPort *port, ScmObj next_paths,
                                     ScmObj env, int flags);
SCM_EXTERN ScmObj Scm_VMLoad(ScmString *file, ScmObj paths, ScmObj env,
			     int flags);

#if defined(GAUCHE_API_0_8_10) || defined(GAUCHE_API_0_9) || defined(LIBGAUCHE_BODY)
SCM_EXTERN int Scm_LoadFromPort(ScmPort *port, int flags, ScmLoadPacket *p);
SCM_EXTERN int Scm_Load(const char *file, int flags, ScmLoadPacket *p);
#else  /*!GAUCHE_API_0_9*/
#define Scm_LoadFromPort(port, flags)  Scm__LoadFromPortCompat(port, flags) 
#define Scm_Load(port, flags)          Scm__LoadCompat(port, flags) 
SCM_EXTERN void Scm__LoadFromPortCompat(ScmPort *port, int flags);
SCM_EXTERN int  Scm__LoadCompat(const char *file, int flags);
#endif /*!GAUCHE_API_0_9*/

/*=================================================================
 * Load path management
 */

SCM_EXTERN ScmObj Scm_GetLoadPath(void);
SCM_EXTERN ScmObj Scm_AddLoadPath(const char *cpath, int afterp);

/*=================================================================
 * Dynamic Loading
 */

SCM_EXTERN ScmObj Scm_DynLoad(ScmString *path, ScmObj initfn, int export_);

/*=================================================================
 * Require & Provide
 */

#if defined(GAUCHE_API_0_9) || defined(LIBGAUCHE_BODY)
SCM_EXTERN int Scm_Require(ScmObj feature, int flags, ScmLoadPacket *p);
#else  /*!GAUCHE_API_0_9*/
#define Scm_Require(feature) Scm__RequireCompat(feature)
SCM_EXTERN ScmObj Scm__RequireCompat(ScmObj feature);
#endif /*!GAUCHE_API_0_9*/
SCM_EXTERN ScmObj Scm_Provide(ScmObj feature);
SCM_EXTERN int    Scm_ProvidedP(ScmObj feature);

/*=================================================================
 * Autoloads
 */
struct ScmAutoloadRec {
    SCM_HEADER;
    ScmSymbol *name;            /* variable to be autoloaded */
    ScmModule *module;          /* where the binding should be inserted.
                                   this is where autoload is defined. */
    ScmString *path;            /* file to load */
    ScmSymbol *import_from;     /* module to be imported after loading */
    ScmModule *import_to;       /* module to where import_from should be
                                   imported */
                                /* The fields above will be set up when
                                   the autoload object is created, and never
                                   be modified. */

    int loaded;                 /* The flag that indicates this autoload
                                   is resolved, and value field contains
                                   the resolved value.  Once the autoload
                                   goes into "loaded" status, no field
                                   should be changed. */
    ScmObj value;               /* The resolved value */
    ScmInternalMutex mutex;     /* mutex to resolve this autoload */
    ScmInternalCond cv;         /* ... and condition variable. */
    ScmVM *locker;              /* The thread that is resolving the autoload.*/
};

SCM_CLASS_DECL(Scm_AutoloadClass);
#define SCM_CLASS_AUTOLOAD      (&Scm_AutoloadClass)
#define SCM_AUTOLOADP(obj)      SCM_XTYPEP(obj, SCM_CLASS_AUTOLOAD)
#define SCM_AUTOLOAD(obj)       ((ScmAutoload*)(obj))

SCM_EXTERN ScmObj Scm_MakeAutoload(ScmModule *where,
                                   ScmSymbol *name, ScmString *path,
				   ScmSymbol *import_from);
SCM_EXTERN void   Scm_DefineAutoload(ScmModule *where, ScmObj file_or_module,
                                     ScmObj list);
SCM_EXTERN ScmObj Scm_LoadAutoload(ScmAutoload *autoload);

#endif /* GAUCHE_LOAD_H */

