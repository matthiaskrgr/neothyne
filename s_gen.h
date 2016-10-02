#ifndef S_GEN_HDR
#define S_GEN_HDR

#include "s_object.h"

namespace s {

struct Gen {
    static size_t newBlock(Gen *gen);

    static void terminate(Gen *gen);

    static Slot addAccess(Gen *gen, Slot objectSlot, Slot keySlot);
    static void addAssign(Gen *gen, Slot objectSlot, Slot keySlot, Slot slot, AssignType assignType);

    static void addCloseObject(Gen *gen, Slot objectSlot);

    static Slot addGetContext(Gen *gen);

    static Slot addNewObject(Gen *gen, Slot parentSlot);
    static Slot addNewIntObject(Gen *gen, Slot contextSlot, int value);
    static Slot addNewFloatObject(Gen *gen, Slot contextSlot, float value);
    static Slot addNewArrayObject(Gen *gen, Slot contextSlot);
    static Slot addNewStringObject(Gen *gen, Slot contextSlot, const char *value);
    static Slot addNewClosureObject(Gen *gen, Slot contextSlot, UserFunction *function);

    static Slot addCall(Gen *gen, Slot functionSlot, Slot thisSlot, Slot *arguments, size_t count);
    static Slot addCall(Gen *gen, Slot functionSlot, Slot thisSlot);
    static Slot addCall(Gen *gen, Slot functionSlot, Slot thisSlot, Slot argument0);
    static Slot addCall(Gen *gen, Slot functionSlot, Slot thisSlot, Slot argument0, Slot argument1);

    static void addTestBranch(Gen *gen, Slot testSlot, size_t **trueBranch, size_t **falseBranch);
    static void addBranch(Gen *gen, size_t **branch);

    static void addReturn(Gen *gen, Slot slot);

    static UserFunction *buildFunction(Gen *gen);

    static UserFunction *optimize(UserFunction *function);

    static UserFunction *inlinePass(UserFunction *function, bool *primitiveSlots);

private:
    friend struct Parser;

    static void addInstruction(Gen *gen, Instruction *instruction);

    const char *m_name;
    const char **m_arguments;
    size_t m_count;
    Slot m_scope;
    Slot m_slot;
    bool m_blockTerminated;
    FunctionBody m_body;
};

}

#endif