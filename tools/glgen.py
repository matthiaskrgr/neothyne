#!/usr/bin/env python

import textwrap, sys, getopt
from proto import *

# Data types:
#   name    - OpenGL data type
#   format  - Format specifier to print `name' data type
#   promote - Data type it promotes to for va_arg
#   spec    - Unique specifier for it
types = [
    {'name': 'GLvoid',     'format': '',     'promote': '',             'spec': '0' },
    {'name': 'GLchar',     'format': '%c',   'promote': 'int',          'spec': '1' },
    {'name': 'GLenum',     'format': '0x%X', 'promote': 'unsigned int', 'spec': '2' },
    {'name': 'GLboolean',  'format': '%c',   'promote': 'int',          'spec': '3' },
    {'name': 'GLbitfield', 'format': '%u',   'promote': 'unsigned int', 'spec': '4' },
    {'name': 'GLbyte',     'format': '%x',   'promote': 'int',          'spec': '5' },
    {'name': 'GLshort',    'format': '%d',   'promote': 'int',          'spec': '6' },
    {'name': 'GLint',      'format': '%d',   'promote': 'int',          'spec': '7' },
    {'name': 'GLsizei',    'format': '%d',   'promote': 'int',          'spec': '8' },
    {'name': 'GLubyte',    'format': '%X',   'promote': 'unsigned int', 'spec': '9' },
    {'name': 'GLushort',   'format': '%u',   'promote': 'unsigned int', 'spec': 'a' },
    {'name': 'GLuint',     'format': '%u',   'promote': 'unsigned int', 'spec': 'b' },
    {'name': 'GLfloat',    'format': '%.2f', 'promote': 'double',       'spec': 'c' },
    {'name': 'GLclampf',   'format': '%f',   'promote': 'double',       'spec': 'd' },
    {'name': 'GLintptr',   'format': '%p',   'promote': 'intptr_t',     'spec': 'e' },
    {'name': 'GLsizeiptr', 'format': '%p',   'promote': 'intptr_t',     'spec': 'f' }
]

# Read a list of extensions from an extension file and return a list of strings
# of such extensions
def readExtensions(extensionFile):
    extensions = []
    with open(extensionFile) as data:
        for line in data.readlines():
            extensions.append(line.strip())
    return extensions

# Print the GL_CHECK call with the right type spec string
def printCheck(stream, function):
    # Generate the spec string
    specs = ''
    for formal in function.formals:
        base = ''.join([i for i in formal[0].split() if i not in ['const']]).strip('*')
        count = formal[0].count('*')
        if count:
            specs += '*'
            if count != 1:
                base = 'GLvoid'
        specs += ''.join([x['spec'] for x in types if x['name'] == base])
    if len(function.formals):
        stream.write('    GL_CHECK("%s", ' % (specs))
        printFormals(stream, function, True, False, False)
        stream.write(');\n')
    else:
        stream.write('    GL_CHECK("",0);\n')

def infoTag(function):
    return ' GL_INFOP' if len(function.formals) else 'GL_INFO'

def genHeader(functionList, extensionList, headerFile):
    with open(headerFile, 'w') as header:
        # Begin the header
        header.write(textwrap.dedent("""\
        // file automatically generated by %s
        #ifndef R_COMMON_HDR
        #define R_COMMON_HDR
        #include <SDL2/SDL_opengl.h>
        #include <stdint.h>

        namespace u {
        template <typename T>
        struct set;
        }

        #ifdef DEBUG_GL
        #   define GL_INFO const char *file, size_t line
        #   define GL_INFOP , GL_INFO
        #else
        #   define GL_INFO
        #   define GL_INFOP
        #endif

        #define ATTRIB_OFFSET(X) ((const GLvoid *)(sizeof(GLfloat) * (X)))

        namespace gl {

        enum : size_t {
        """) % (__file__))

        for i, e in enumerate(extensionList):
            header.write(('    %s\n' if i == len(extensionList) - 1 else '    %s,\n') % (e))
        header.write(textwrap.dedent("""\
        };

        void init();
        const char *extensionString(size_t what);
        const u::set<size_t> &extensions();
        bool has(size_t ext);

        """))
        # Generate the function prototypes
        for function in functionList:
            printPrototype(header, function, infoTag(function), True)

        header.write(textwrap.dedent("""\

        }
        #if defined(DEBUG_GL) && !defined(R_COMMON_NO_DEFINES)
        """))
        # Generate the wrapper macros
        largest = max(len(x.name) for x in functionList)
        for f in functionList:
            header.write('#   define %s(...) %s(%s __FILE__, __LINE__)\n'
                % (f.name,
                   f.name.rjust(largest),
                   '__VA_ARGS__,' if len(f.formals) else '/* no arg */')
            )
        # Finish the wrapper macros
        header.write(textwrap.dedent("""\
        #endif
        #endif
        """))

def genSource(functionList, extensionList, sourceFile):
    with open(sourceFile, 'w') as source:
        # Begin the source
        source.write(textwrap.dedent("""\
        // File automatically generated by %s
        #include <stdarg.h>
        #define R_COMMON_NO_DEFINES
        #include "r_common.h"

        #include "u_string.h"
        #include "u_set.h"
        #include "u_misc.h"

        #include "engine.h"

        #ifndef APIENTRY
        #   ifdef _WIN32
        #       define APIENTRY __stdcall
        #   else
        #       define APIENTRY
        #   endif
        #endif

        #ifndef APIENTRYP
        #   define APIENTRYP APIENTRY *
        #endif

        #ifdef DEBUG_GL
        #   define GL_CHECK(SPEC, ...) debugCheck((SPEC), __func__, file, line, __VA_ARGS__)
        #else
        #   define GL_CHECK(...)
        #endif

        """) % (__file__))
        # Generate the PFNGL typedefs
        for f in functionList:
            source.write('typedef %s (APIENTRYP MYPFNGL%sPROC)'
                % (f.type, f.name.upper()))
            printFormals(source, f, False, True)
            source.write(';\n')
        source.write('\n')
        largest = max(len(x.name) for x in functionList)
        for f in functionList:
            fill = largest - len(f.name)
            source.write('static MYPFNGL%sPROC %s gl%s_ %s= nullptr;\n'
                % (f.name.upper(),
                   ''.rjust(fill),
                   f.name,
                   ''.rjust(fill))
            )
        # Begin DEBUG section
        source.write(textwrap.dedent("""\

        #ifdef DEBUG_GL
        template <char C, typename T>
        u::string stringize(T, char base='?');

        """))
        # Generate all the stringize functions
        for t in types[1:]:
            source.write('template<>\n')
            source.write('u::string stringize<\'%s\', %s>(%s value, char) {\n'
                % (t['spec'], t['name'], t['name']))
            source.write('    return u::format("%s=%s", value);\n'
                % (t['name'], t['format']))
            source.write('}\n')
        # Generate the beginning of the pointer stringize specialization
        source.write(textwrap.dedent("""\
        template <>
        u::string stringize<'*', void *>(void *value, char base) {
            switch (base) {
                case '?': return "unknown";
        """))
        # Generate all cases
        for t in types:
            source.write('        case \'%s\': return u::format("%s*=%%p", value);\n'
                % (t['spec'], t['name']))
        # Finish the pointer stringize specialization and emit the
        # debugErrorString function and beginning of debugCheck function
        source.write(textwrap.dedent("""\
            }

            return u::format("GLchar*=\\"%s\\"", (const char *)value);
        }

        static const char *debugErrorString(GLenum error) {
            switch (error) {
                case GL_INVALID_ENUM:
                    return "GL_INVALID_ENUM";
                case GL_INVALID_VALUE:
                    return "GL_INVALID_VALUE";
                case GL_INVALID_OPERATION:
                    return "GL_INVALID_OPERATION";
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                    return "GL_INVALID_FRAMEBUFFER_OPERATION";
            }
            return "unknown";
        }

        static void debugCheck(const char *spec, const char *function, const char *file, size_t line, ...) {
            GLenum error = glGetError_();
            if (error == GL_NO_ERROR)
                return;

            va_list va;
            va_start(va, line);
            u::string contents;

            for (const char *s = spec; *s; s++) {
                switch (*s) {
        """))
        # Generate all the switches on the spec string
        for t in types[1:]:
            source.write('            case \'%s\':\n' % (t['spec']))
            source.write('                contents += stringize<\'%s\'>((%s)va_arg(va, %s));\n'
                % (t['spec'], t['name'], t['promote']))
            source.write('                break;\n')
        # Emit the end of debugCheck function
        source.write(textwrap.dedent("""\
                    case '*':
                        contents += stringize<'*'>(va_arg(va, void *), s[1]);
                        s++; // skip basetype spec
                        break;
                }
                if (s[1])
                    contents += ", ";
            }
            va_end(va);
            u::fprint(stderr, "error %s(%s) (%s:%zu) %s\\n", function, contents,
                file, line, debugErrorString(error));
        }
        """))

        source.write(textwrap.dedent("""\
        #endif

        namespace gl {

        static u::set<size_t> gExtensions;
        static const char *kExtensions[] = {
        """))
        for i, e in enumerate(extensionList):
            source.write('    "GL_%s"%s\n' % (e, ',' if i != len(extensionList) - 1 else ''))
        source.write(textwrap.dedent("""\
        };

        const char *extensionString(size_t what) {
            return kExtensions[what];
        }

        const u::set<size_t> &extensions() {
            return gExtensions;
        }

        void init() {
        """))
        for f in functionList:
            fill = largest - len(f.name)
            source.write('    gl%s_%s = (MYPFNGL%sPROC)neoGetProcAddress("gl%s");\n'
                % (f.name, ''.rjust(fill), f.name.upper(), f.name))
        source.write(textwrap.dedent("""\

            if (!glGetIntegerv_ || !glGetStringi_)
                neoFatal("Failed to initialize OpenGL\\n");

            GLint count = 0;
            glGetIntegerv_(GL_NUM_EXTENSIONS, &count);
            for (GLint i = 0; i < count; i++)
                for (size_t j = 0; j < sizeof(kExtensions)/sizeof(*kExtensions); j++)
                    if (!strcmp(kExtensions[j], (const char *)glGetStringi_(GL_EXTENSIONS, i)))
                        gExtensions.insert(j);
        }

        bool has(size_t ext) {
            return gExtensions.find(ext) != gExtensions.end();
        }

        """))
        # Generate the GL function wrappers
        for f in functionList:
            source.write('\n%s %s' % (f.type, f.name))
            printFormals(source, f, True, True, True, infoTag(f))
            source.write(' {\n    ')
            if f.type != 'void':
                # Local backup result and then return it
                source.write('%s result = gl%s_' % (f.type, f.name))
                printFormals(source, f, True, False)
                source.write(';\n')
                printCheck(source, f)
                source.write('    return result;\n}\n')
            else:
                # Just call
                source.write('gl%s_' % (f.name))
                printFormals(source, f, True, False)
                source.write(';\n')
                printCheck(source, f)
                source.write('}\n')
        # End the namespace
        source.write('\n}\n')

def main(argv):
    protos = 'tools/glprotos'
    extens = 'tools/glextensions'
    output = 'r_common'

    functionList = readPrototypes(protos)
    extensionList = readExtensions(extens)

    genHeader(functionList, extensionList, '%s.h' % (output))
    genSource(functionList, extensionList, '%s.cpp' % (output))

if __name__ == "__main__":
    main(sys.argv)
