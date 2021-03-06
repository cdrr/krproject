#include "krutils/kr_utils.h"

#include "kr_calc_tree.h"
#include "kr_calc_parser.h"

/* calctree traversal function definition */
typedef int (*traverse_func)(T_KRCalcTree *node, void *data);


/* variable indentno is used by kr_ruletree_node_print to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=4
#define UNINDENT indentno-=4

/* create a new calctree node */
T_KRCalcTree *kr_calctree_node_new(E_KRCalcKind kind)
{ 
    int i;  
    T_KRCalcTree *t = (T_KRCalcTree *)kr_calloc(sizeof(T_KRCalcTree));
    if (t == NULL) {
        fprintf(stderr, "kr_calloc T_KRCalcTree failed!\n");
        return NULL;
    }
    t->kind = kind;
    t->ind  = KR_VALUE_UNSET;
    t->type = KR_TYPE_UNKNOWN;
    for (i=0; i<MAXCHILDREN; i++) {
        t->child[i] = NULL;
    }
    return t;
}


/* procedure kr_calctree_traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies pre_proc in preorder and post_proc in postorder
 * to tree pointed to by t
 */
static int kr_calctree_traverse(T_KRCalcTree *t, void *data,
                     traverse_func pre_proc, traverse_func post_proc)
{ 
    if (t != NULL) { 
        int i=0;
        int ret= 0;
        if (pre_proc != NULL) {
            if (pre_proc(t, data) != 0) return -1;
        }
        
        for (i=0; i < MAXCHILDREN; i++) {
            ret = kr_calctree_traverse(t->child[i], data, pre_proc, post_proc);
            if (ret != 0) {
                return -1;
            }
        }
        
        if (post_proc != NULL) {
            if (post_proc(t, data) != 0) return -1;
        }
    }
    return 0;
}


/* procedure checkNode performs
 * type checking at a single tree node
 */
static int check_treenode(T_KRCalcTree * t, char *errmsg)
{ 
    /*we don't know the type of some factors, ignore the type checking*/
    int i=0;
    for (i=0; i<MAXCHILDREN; ++i) {
        if (t->child[i] && t->child[i]->type == KR_TYPE_UNKNOWN) {
            return 0;
        }
    }
    
    switch (t->kind)
    { 
        case KR_CALCKIND_ARITH:
            switch(t->op) 
            {
                case KR_CALCOP_PLUS: case KR_CALCOP_SUB: 
                case KR_CALCOP_MUT: case KR_CALCOP_DIV: 
                case KR_CALCOP_MOD:
                    if ((t->child[0]->type != KR_TYPE_INT && 
                                t->child[0]->type != KR_TYPE_DOUBLE) ||
                            (t->child[1]->type != KR_TYPE_INT && 
                             t->child[1]->type != KR_TYPE_DOUBLE)) {
                        sprintf(errmsg, "Arithmetic Op[%d] Type Error[%c][%c].",
                                t->op, t->child[0]->type, t->child[1]->type);
                        return -1;
                    }
                    break;
                default:
                    sprintf(errmsg, "[%d] not an Arithmetic Op.\n", t->op);
                    return -1;
            }
            break;
        case KR_CALCKIND_LOGIC:
            switch(t->op) {
                case KR_CALCOP_LT: case KR_CALCOP_LE: 
                case KR_CALCOP_GT: case KR_CALCOP_GE: 
                case KR_CALCOP_EQ: case KR_CALCOP_NEQ:
                    if ((t->child[0]->type == KR_TYPE_STRING && 
                                t->child[1]->type != KR_TYPE_STRING) ||
                            (t->child[1]->type == KR_TYPE_STRING && 
                             t->child[0]->type != KR_TYPE_STRING)) {
                        sprintf(errmsg, "Logic Op[%d] Type Error[%c][%c].", \
                                t->op, t->child[0]->type, t->child[1]->type);
                        return -1;
                    }
                    break;
                case KR_CALCOP_AND: case KR_CALCOP_OR: 
                    if ((t->child[0]->type != KR_TYPE_INT && 
                                t->child[0]->type != KR_TYPE_BOOL) ||
                            (t->child[1]->type != KR_TYPE_INT && 
                             t->child[1]->type != KR_TYPE_BOOL)) {
                        sprintf(errmsg, "Logic Op[%d] Type Error[%c][%c].", \
                                t->op, t->child[0]->type, t->child[1]->type);
                        return -1;
                    }
                    break;
                case KR_CALCOP_NOT:
                    if (t->child[0]->type != KR_TYPE_INT && 
                            t->child[0]->type != KR_TYPE_BOOL) {
                        sprintf(errmsg, "Logic Op[%d] Type Error[%c].", \
                                t->op, t->child[0]->type);
                        return -1;
                    }
                case KR_CALCOP_BL: case KR_CALCOP_NBL:
                    if ((t->child[1]->kind != KR_CALCKIND_MINT && 
                                t->child[1]->kind != KR_CALCKIND_MFLOAT && 
                                t->child[1]->kind != KR_CALCKIND_MSTRING && 
                                t->child[1]->kind != KR_CALCKIND_SET) ||
                            t->child[0]->type != t->child[1]->type) {
                        sprintf(errmsg, "Logic Op[%d] Type Error[%c][%c][%d].",
                                t->op, t->child[0]->type, t->child[1]->type, 
                                t->child[1]->kind);
                        return -1;        
                    }
                    break;
                case KR_CALCOP_MATCH:
                    if (t->child[0]->type != KR_TYPE_STRING ||
                            t->child[1]->kind != KR_CALCKIND_REGEX) {
                        sprintf(errmsg, "Logic Op[%d] Type Error[%c][%c][%d].",
                                t->op, t->child[0]->type, t->child[1]->type,
                                t->child[1]->kind);
                        return -1;
                    }
                    break;
                default:
                    sprintf(errmsg, "[%d] is not an Logical Op.\n", t->op);
                    return -1;
            }
            break;
    }
    return 0;
}

/* procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
int kr_calctree_check(T_KRCalcTree *root, char *errmsg)
{
    return kr_calctree_traverse(root, errmsg, NULL, \
                     (traverse_func )check_treenode);
}


/*do the arithmetic evaluation*/
static int _kr_evaluate_arith(T_KRCalcTree *t)
{
    t->ind = KR_VALUE_UNSET;
    double d0=0.0;
    double d1=0.0;
    double d=0.0;
    
    /*promote left value to double*/
    switch(t->child[0]->type) 
    {
        case KR_TYPE_INT: 
            d0 = (double )t->child[0]->attr.val.i; 
            break;
        case KR_TYPE_DOUBLE: 
            d0 = (double )t->child[0]->attr.val.d; 
            break;
        default: return -1;       
    }
    
    /*promote right value to double*/
    switch(t->child[1]->type) 
    {
        case KR_TYPE_INT: 
            d1 = (double )t->child[1]->attr.val.i; 
            break;
        case KR_TYPE_DOUBLE: 
            d1 = (double )t->child[1]->attr.val.d; 
            break;
        default: return -1;       
    }
    
    /*do the arithmetic operation*/
    switch(t->op) 
    {
        case KR_CALCOP_PLUS: d = d0 + d1; break;
        case KR_CALCOP_SUB:  d = d0 - d1; break;
        case KR_CALCOP_MUT:  d = d0 * d1; break;
        case KR_CALCOP_DIV:  d = d0 / d1; break;
        case KR_CALCOP_MOD:  d = (int )d0 % (int )d1; break;
        default: return -1;      
    }
    
    /*set result*/
    switch(t->type) 
    {
        case KR_TYPE_INT: 
            t->attr.val.i = (int ) d; 
            break;
        case KR_TYPE_DOUBLE: 
            t->attr.val.d = (double ) d; 
            break;
        default: return -1;
    }
    
    t->ind = KR_VALUE_SETED;
    return 0;
}


/*do the logical evaluation*/
static int _kr_evaluate_logic(T_KRCalcTree *t)
{
    t->ind = KR_VALUE_UNSET;
    kr_bool b = FALSE;
    double d0=0.0;
    double d1=0.0;
    
    /*promote left value to double*/
    if (t->child[0]) {
        switch(t->child[0]->type) 
        {
            case KR_TYPE_BOOL:
                d0 = (double )t->child[0]->attr.val.b; 
                break;
            case KR_TYPE_INT: 
                d0 = (double )t->child[0]->attr.val.i; 
                break;
            case KR_TYPE_DOUBLE: 
                d0 = (double )t->child[0]->attr.val.d; 
                break;
        }
    }
    
    /*promote right value to double*/
    if (t->child[1]) {
        switch(t->child[1]->type) 
        {
            case KR_TYPE_BOOL:
                d1 = (double )t->child[1]->attr.val.b; 
                break;
            case KR_TYPE_INT: 
                d1 = (double )t->child[1]->attr.val.i; 
                break;
            case KR_TYPE_DOUBLE: 
                d1 = (double )t->child[1]->attr.val.d; 
                break;
        }
    }

    /*do the logic operation*/
    switch(t->op) 
    {
        case KR_CALCOP_LT:
            if (t->child[0]->type == KR_TYPE_STRING && 
                    t->child[1]->type == KR_TYPE_STRING) {
                b = (strcmp(t->child[0]->attr.val.s, 
                            t->child[1]->attr.val.s) < 0);
            } else if (t->child[0]->type != KR_TYPE_STRING &&
                    t->child[1]->type != KR_TYPE_STRING) {
                b = (d0 < d1)? TRUE:FALSE;
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_LE:
            if (t->child[0]->type == KR_TYPE_STRING && 
                    t->child[1]->type == KR_TYPE_STRING) {
                b = (strcmp(t->child[0]->attr.val.s, 
                            t->child[1]->attr.val.s) <= 0);
            } else if (t->child[0]->type != KR_TYPE_STRING &&
                    t->child[1]->type != KR_TYPE_STRING) {
                b = (d0 <= d1)? TRUE:FALSE;
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_GT:
            if (t->child[0]->type == KR_TYPE_STRING && 
                    t->child[1]->type == KR_TYPE_STRING) {
                b = (strcmp(t->child[0]->attr.val.s, 
                            t->child[1]->attr.val.s) > 0);
            } else if (t->child[0]->type != KR_TYPE_STRING &&
                    t->child[1]->type != KR_TYPE_STRING) {
                b = (d0 > d1)? TRUE:FALSE;
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_GE:
            if (t->child[0]->type == KR_TYPE_STRING && 
                    t->child[1]->type == KR_TYPE_STRING) {
                b = (strcmp(t->child[0]->attr.val.s, 
                            t->child[1]->attr.val.s) >= 0);
            } else if (t->child[0]->type != KR_TYPE_STRING &&
                    t->child[1]->type != KR_TYPE_STRING) {
                b = (d0 >= d1)? TRUE:FALSE;
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_EQ: 
            if (t->child[0]->type == KR_TYPE_STRING && 
                    t->child[1]->type == KR_TYPE_STRING) {
                b = (strcmp(t->child[0]->attr.val.s, 
                            t->child[1]->attr.val.s) == 0);
            } else if (t->child[0]->type != KR_TYPE_STRING &&
                    t->child[1]->type != KR_TYPE_STRING) {
                b = (d0 == d1)? TRUE:FALSE;
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_NEQ:
            if (t->child[0]->type == KR_TYPE_STRING && 
                    t->child[1]->type == KR_TYPE_STRING) {
                b = strcmp(t->child[0]->attr.val.s, t->child[1]->attr.val.s);
            } else if (t->child[0]->type != KR_TYPE_STRING &&
                    t->child[1]->type != KR_TYPE_STRING) {
                b = (d0 != d1)? TRUE:FALSE;
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_AND: 
            if ((t->child[0]->type == KR_TYPE_INT || 
                        t->child[0]->type == KR_TYPE_BOOL) &&
                    (t->child[1]->type == KR_TYPE_INT || 
                     t->child[1]->type == KR_TYPE_BOOL)) {
                b = ((int )d0 && (int )d1);
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_OR:  
            if ((t->child[0]->type == KR_TYPE_INT || 
                        t->child[0]->type == KR_TYPE_BOOL) &&
                    (t->child[1]->type == KR_TYPE_INT || 
                     t->child[1]->type == KR_TYPE_BOOL)) {
                b = ((int )d0 || (int )d1);
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_NOT: 
            if (t->child[0]->type == KR_TYPE_BOOL ||
                    t->child[0]->type == KR_TYPE_INT) {
                b = (!(int )d0 );
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_BL:
            if (t->child[1]->kind == KR_CALCKIND_MINT || 
                    t->child[1]->kind == KR_CALCKIND_MFLOAT || 
                    t->child[1]->kind == KR_CALCKIND_MSTRING || 
                    t->child[1]->kind == KR_CALCKIND_SET) {
                if (t->child[0]->type == KR_TYPE_STRING &&
                        t->child[1]->type == KR_TYPE_STRING) {
                    b = kr_hashset_search(t->child[1]->attr.set, \
                            t->child[0]->attr.val.s);
                } else if(t->child[0]->type == t->child[1]->type) {
                    b = kr_hashset_search(t->child[1]->attr.set, \
                            &t->child[0]->attr.val);
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
            break;
        case KR_CALCOP_NBL:
            if (t->child[1]->kind == KR_CALCKIND_MINT || 
                    t->child[1]->kind == KR_CALCKIND_MFLOAT || 
                    t->child[1]->kind == KR_CALCKIND_MSTRING || 
                    t->child[1]->kind == KR_CALCKIND_SET) {
                if (t->child[0]->type == KR_TYPE_STRING &&
                        t->child[1]->type == KR_TYPE_STRING) {
                    b = !kr_hashset_search(t->child[1]->attr.set, \
                            t->child[0]->attr.val.s);
                } else if(t->child[0]->type == t->child[1]->type) {
                    b = !kr_hashset_search(t->child[1]->attr.set, \
                            &t->child[0]->attr.val);
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
            break;    
        case KR_CALCOP_MATCH:
            if (t->child[0]->type == KR_TYPE_STRING &&
                    t->child[1]->kind == KR_CALCKIND_REGEX) {
                b = kr_regex_execute(t->child[1]->attr.regex, \
                        t->child[0]->attr.val.s);
            } else {
                return -1;
            }
            break;
    }
        
    /*set result*/
    t->ind = KR_VALUE_SETED;
    t->attr.val.b = b;
    
    return 0;
}

/*do the extern identifier evaluation*/
static int _kr_evaluate_extern(T_KRCalcTree *t, T_KRCalc *krcalc)
{
    t->ind = KR_VALUE_UNSET;
    /*type only set once for optimization*/
    if (t->type == KR_TYPE_UNKNOWN) {
        t->type = krcalc->get_type_cb(t->kind, t->id, krcalc->data);
        if (t->type == KR_TYPE_UNKNOWN) {
            KR_LOG(KR_LOGERROR, "kind[%d],id[%d] type unknown", \
                    t->kind, t->id);
            return -1;
        }
    }
    
    void *val = krcalc->get_value_cb(t->kind, t->id, krcalc->data);
    if (val != NULL) {
        switch(t->type) 
        {
            case KR_TYPE_BOOL:
                t->attr.val.b = *(kr_bool *)val;
                KR_LOG(KR_LOGDEBUG, "type[%c] value[%d]", \
                        t->type, t->attr.val.b);
                break;
            case KR_TYPE_INT:
                t->attr.val.i = *(kr_int *)val;
                KR_LOG(KR_LOGDEBUG, "type[%c] value[%d]", \
                        t->type, t->attr.val.i);
                break;
            case KR_TYPE_DOUBLE:
                t->attr.val.d = *(kr_double *)val;
                KR_LOG(KR_LOGDEBUG, "type[%c] value[%lf]", \
                        t->type, t->attr.val.d);
                break;
            case KR_TYPE_STRING:
                t->attr.val.s = (kr_string )val;
                KR_LOG(KR_LOGDEBUG, "type[%c] value[%s]", \
                        t->type, t->attr.val.s);
                break;
            default:
                KR_LOG(KR_LOGERROR, "unsupported type[%c]", t->type);
                return -1;
        }
        t->ind = KR_VALUE_SETED;
    }
    return 0;
}

/* evaluate an calculator tree node
 */
static int kr_calctree_node_eval(T_KRCalcTree *t, T_KRCalc *krcalc)
{
    int ret = 0;
    switch (t->kind) 
    { 
        case KR_CALCKIND_ARITH: 
            ret=_kr_evaluate_arith(t);
            break;
        case KR_CALCKIND_LOGIC: 
            ret=_kr_evaluate_logic(t);
            break;
        case KR_CALCKIND_CID:
        case KR_CALCKIND_FID:
        case KR_CALCKIND_SID:
        case KR_CALCKIND_DID:
            ret=_kr_evaluate_extern(t, krcalc);
            break;
    }
    return ret;
}


/* evaluate the calculator */
int kr_calctree_eval(T_KRCalcTree *t, void *param)
{
    if (t != NULL) {   
        int i = 0;      
        for (i = 0; i < MAXCHILDREN && t->child[i]; i++) {
            if (kr_calctree_eval(t->child[i], param) != 0) {
                return -1;
            } else if (t->child[i]->ind != KR_VALUE_SETED) {
                return 0;
            } else if ((t->op == KR_CALCOP_AND) && 
                       (t->child[i]->type == KR_TYPE_BOOL) && 
                       (!t->child[i]->attr.val.b)) {
                t->attr.val.b = FALSE;
                t->ind = KR_VALUE_SETED;
                return 0;
            } else if ((t->op == KR_CALCOP_OR) && 
                       (t->child[i]->type == KR_TYPE_BOOL) && 
                       (t->child[i]->attr.val.b)) {
                t->attr.val.b = TRUE;
                t->ind = KR_VALUE_SETED;
                return 0;
            }
        }
        
        if (kr_calctree_node_eval(t, param) == -1) {
            return -1;
        }
    }
    return 0;
}


/* print operator op, refer to the lexer file's definition
 */
static void print_operator(FILE *printer, E_KRCalcOp op)
{ 
    switch (op)
    {
        case KR_CALCOP_PLUS: fprintf(printer,"+\n"); break;
        case KR_CALCOP_SUB: fprintf(printer,"-\n"); break;
        case KR_CALCOP_MUT: fprintf(printer,"*\n"); break;
        case KR_CALCOP_DIV: fprintf(printer,"/\n"); break;
        case KR_CALCOP_MOD: fprintf(printer,"%%\n"); break;
        case KR_CALCOP_LT: fprintf(printer,"<\n"); break;
        case KR_CALCOP_LE: fprintf(printer,"<=\n"); break;
        case KR_CALCOP_GT: fprintf(printer,">\n"); break;
        case KR_CALCOP_GE: fprintf(printer,">=\n"); break;
        case KR_CALCOP_EQ: fprintf(printer,"==\n"); break;
        case KR_CALCOP_NEQ: fprintf(printer,"!=\n"); break;
        case KR_CALCOP_BL: fprintf(printer,"@@\n"); break;
        case KR_CALCOP_NBL: fprintf(printer,"!@\n"); break;
        case KR_CALCOP_AND: fprintf(printer,"&&\n"); break;
        case KR_CALCOP_OR: fprintf(printer,"||\n"); break;
        case KR_CALCOP_NOT: fprintf(printer,"!\n"); break;
        case KR_CALCOP_MATCH: fprintf(printer,"##\n"); break;
        default: /* should never happen */
            fprintf(printer, "Unknown op: %d\n", op);
    }
}

/* preprocess of printing calctree node  */
static int print_treenode_pre(T_KRCalcTree *node, void *data)
{ 
    int i = 0;
    FILE *printer = data;
    
    INDENT;
    for (i=0; i<indentno; i++)
        fprintf(printer, " ");
        
    switch(node->kind){
        case KR_CALCKIND_ARITH:
            fprintf(printer,"Arith Op: ");
            print_operator(printer, node->op);
            break;
        case KR_CALCKIND_LOGIC:    
            fprintf(printer,"Logic Op: ");
            print_operator(printer, node->op);
            break;
        case KR_CALCKIND_INT:
            fprintf(printer,"Num_Const: %d\n",node->attr.val.i);
            break;
        case KR_CALCKIND_FLOAT:
            fprintf(printer,"FNum_Const: %f\n",node->attr.val.d);
            break;
        case KR_CALCKIND_STRING:
            fprintf(printer,"Char_Const: %s\n",node->attr.val.s);
            break;
        case KR_CALCKIND_CID:
            fprintf(printer,"Cid: %d\n",node->id);
            break;
        case KR_CALCKIND_FID:
            fprintf(printer,"Fid: %d\n",node->id);
            break;
        case KR_CALCKIND_SID:
            fprintf(printer,"Sid: %d\n",node->id);
            break;
        case KR_CALCKIND_DID:
            fprintf(printer,"Did: %d\n",node->id);
            break;    
        case KR_CALCKIND_HID:
            fprintf(printer,"Hid: %d\n",node->id);
            break;    
        case KR_CALCKIND_SET:
            fprintf(printer,"Set: %d\n",node->id);
            break;
        case KR_CALCKIND_MINT:    
        case KR_CALCKIND_MFLOAT:    
        case KR_CALCKIND_MSTRING:    
            fprintf(printer,"Multi: %s\n",node->attr.set->name);
            break;
        case KR_CALCKIND_REGEX:
            fprintf(printer,"Regex: %s\n",node->attr.regex->pattern);
            break;    
        default:
            fprintf(printer,"Unknown kind: %d\n",node->kind);
            return -1;
    }
    return 0;
}


/* postprocess of printing calctree node  */
static int print_treenode_post(T_KRCalcTree *node, void *data)
{ 
    UNINDENT;
    return 0;
}

/* prints a calctree to the 
 * printer file using indentation to indicate subtrees
 */
void kr_calctree_dump(T_KRCalcTree *root, FILE *fp)
{
    kr_calctree_traverse(root, fp, print_treenode_pre, print_treenode_post);
}


/* free this calctree node and free his allocated subvalue */
static int kr_calctree_node_free(T_KRCalcTree *node, void *data)
{ 
    if (node != NULL) {
        switch (node->kind) {
            case KR_CALCKIND_STRING:
                kr_free(node->attr.val.s); 
                break;
            case KR_CALCKIND_SET:
            case KR_CALCKIND_MINT:
            case KR_CALCKIND_MFLOAT:
            case KR_CALCKIND_MSTRING:
                kr_hashset_destroy(node->attr.set); 
                break;
            case KR_CALCKIND_REGEX:
                kr_regex_free(node->attr.regex); 
                break;
        }
        kr_free(node); node = NULL;
    }
    
    return 0;
}


/* free a calctree */
void kr_calctree_free(T_KRCalcTree *root)
{
    kr_calctree_traverse(root, NULL, kr_calctree_node_free, NULL);
}
