/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sw=4 et tw=99:
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "frontend/FoldConstants.h"

#include "jslibmath.h"

#include "frontend/CodeGenerator.h"
#include "frontend/ParseNode.h"

#if JS_HAS_XML_SUPPORT
#include "jsxml.h"
#endif

using namespace js;

static JSParseNode *
ContainsStmt(JSParseNode *pn, TokenKind tt)
{
    JSParseNode *pn2, *pnt;

    if (!pn)
        return NULL;
    if (pn->isKind(tt))
        return pn;
    switch (pn->getArity()) {
      case PN_LIST:
        for (pn2 = pn->pn_head; pn2; pn2 = pn2->pn_next) {
            pnt = ContainsStmt(pn2, tt);
            if (pnt)
                return pnt;
        }
        break;
      case PN_TERNARY:
        pnt = ContainsStmt(pn->pn_kid1, tt);
        if (pnt)
            return pnt;
        pnt = ContainsStmt(pn->pn_kid2, tt);
        if (pnt)
            return pnt;
        return ContainsStmt(pn->pn_kid3, tt);
      case PN_BINARY:
        /*
         * Limit recursion if pn is a binary expression, which can't contain a
         * var statement.
         */
        if (!pn->isOp(JSOP_NOP))
            return NULL;
        pnt = ContainsStmt(pn->pn_left, tt);
        if (pnt)
            return pnt;
        return ContainsStmt(pn->pn_right, tt);
      case PN_UNARY:
        if (!pn->isOp(JSOP_NOP))
            return NULL;
        return ContainsStmt(pn->pn_kid, tt);
      case PN_NAME:
        return ContainsStmt(pn->maybeExpr(), tt);
      case PN_NAMESET:
        return ContainsStmt(pn->pn_tree, tt);
      default:;
    }
    return NULL;
}

/*
 * Fold from one constant type to another.
 * XXX handles only strings and numbers for now
 */
static JSBool
FoldType(JSContext *cx, JSParseNode *pn, TokenKind type)
{
    if (!pn->isKind(type)) {
        switch (type) {
          case TOK_NUMBER:
            if (pn->isKind(TOK_STRING)) {
                jsdouble d;
                if (!ToNumber(cx, StringValue(pn->pn_atom), &d))
                    return JS_FALSE;
                pn->pn_dval = d;
                pn->setKind(TOK_NUMBER);
                pn->setOp(JSOP_DOUBLE);
            }
            break;

          case TOK_STRING:
            if (pn->isKind(TOK_NUMBER)) {
                JSString *str = js_NumberToString(cx, pn->pn_dval);
                if (!str)
                    return JS_FALSE;
                pn->pn_atom = js_AtomizeString(cx, str);
                if (!pn->pn_atom)
                    return JS_FALSE;
                pn->setKind(TOK_STRING);
                pn->setOp(JSOP_STRING);
            }
            break;

          default:;
        }
    }
    return JS_TRUE;
}

/*
 * Fold two numeric constants.  Beware that pn1 and pn2 are recycled, unless
 * one of them aliases pn, so you can't safely fetch pn2->pn_next, e.g., after
 * a successful call to this function.
 */
static JSBool
FoldBinaryNumeric(JSContext *cx, JSOp op, JSParseNode *pn1, JSParseNode *pn2,
                  JSParseNode *pn, JSTreeContext *tc)
{
    jsdouble d, d2;
    int32 i, j;

    JS_ASSERT(pn1->isKind(TOK_NUMBER) && pn2->isKind(TOK_NUMBER));
    d = pn1->pn_dval;
    d2 = pn2->pn_dval;
    switch (op) {
      case JSOP_LSH:
      case JSOP_RSH:
        i = js_DoubleToECMAInt32(d);
        j = js_DoubleToECMAInt32(d2);
        j &= 31;
        d = (op == JSOP_LSH) ? i << j : i >> j;
        break;

      case JSOP_URSH:
        j = js_DoubleToECMAInt32(d2);
        j &= 31;
        d = js_DoubleToECMAUint32(d) >> j;
        break;

      case JSOP_ADD:
        d += d2;
        break;

      case JSOP_SUB:
        d -= d2;
        break;

      case JSOP_MUL:
        d *= d2;
        break;

      case JSOP_DIV:
        if (d2 == 0) {
#if defined(XP_WIN)
            /* XXX MSVC miscompiles such that (NaN == 0) */
            if (JSDOUBLE_IS_NaN(d2))
                d = js_NaN;
            else
#endif
            if (d == 0 || JSDOUBLE_IS_NaN(d))
                d = js_NaN;
            else if (JSDOUBLE_IS_NEG(d) != JSDOUBLE_IS_NEG(d2))
                d = js_NegativeInfinity;
            else
                d = js_PositiveInfinity;
        } else {
            d /= d2;
        }
        break;

      case JSOP_MOD:
        if (d2 == 0) {
            d = js_NaN;
        } else {
            d = js_fmod(d, d2);
        }
        break;

      default:;
    }

    /* Take care to allow pn1 or pn2 to alias pn. */
    if (pn1 != pn)
        RecycleTree(pn1, tc);
    if (pn2 != pn)
        RecycleTree(pn2, tc);
    pn->setKind(TOK_NUMBER);
    pn->setOp(JSOP_DOUBLE);
    pn->setArity(PN_NULLARY);
    pn->pn_dval = d;
    return JS_TRUE;
}

#if JS_HAS_XML_SUPPORT

static JSBool
FoldXMLConstants(JSContext *cx, JSParseNode *pn, JSTreeContext *tc)
{
    TokenKind tt;
    JSParseNode **pnp, *pn1, *pn2;
    JSString *accum, *str;
    uint32 i, j;

    JS_ASSERT(pn->isArity(PN_LIST));
    tt = pn->getKind();
    pnp = &pn->pn_head;
    pn1 = *pnp;
    accum = NULL;
    str = NULL;
    if ((pn->pn_xflags & PNX_CANTFOLD) == 0) {
        if (tt == TOK_XMLETAGO)
            accum = cx->runtime->atomState.etagoAtom;
        else if (tt == TOK_XMLSTAGO || tt == TOK_XMLPTAGC)
            accum = cx->runtime->atomState.stagoAtom;
    }

    /*
     * GC Rooting here is tricky: for most of the loop, |accum| is safe via
     * the newborn string root. However, when |pn2->pn_type| is TOK_XMLCDATA,
     * TOK_XMLCOMMENT, or TOK_XMLPI it is knocked out of the newborn root.
     * Therefore, we have to add additonal protection from GC nesting under
     * js_ConcatStrings.
     */
    for (pn2 = pn1, i = j = 0; pn2; pn2 = pn2->pn_next, i++) {
        /* The parser already rejected end-tags with attributes. */
        JS_ASSERT(tt != TOK_XMLETAGO || i == 0);
        switch (pn2->getKind()) {
          case TOK_XMLATTR:
            if (!accum)
                goto cantfold;
            /* FALL THROUGH */
          case TOK_XMLNAME:
          case TOK_XMLSPACE:
          case TOK_XMLTEXT:
          case TOK_STRING:
            if (pn2->isArity(PN_LIST))
                goto cantfold;
            str = pn2->pn_atom;
            break;

          case TOK_XMLCDATA:
            str = js_MakeXMLCDATAString(cx, pn2->pn_atom);
            if (!str)
                return JS_FALSE;
            break;

          case TOK_XMLCOMMENT:
            str = js_MakeXMLCommentString(cx, pn2->pn_atom);
            if (!str)
                return JS_FALSE;
            break;

          case TOK_XMLPI:
            str = js_MakeXMLPIString(cx, pn2->pn_pitarget, pn2->pn_pidata);
            if (!str)
                return JS_FALSE;
            break;

          cantfold:
          default:
            JS_ASSERT(*pnp == pn1);
            if ((tt == TOK_XMLSTAGO || tt == TOK_XMLPTAGC) &&
                (i & 1) ^ (j & 1)) {
#ifdef DEBUG_brendanXXX
                printf("1: %d, %d => ", i, j);
                if (accum)
                    FileEscapedString(stdout, accum, 0);
                else
                    fputs("NULL", stdout);
                fputc('\n', stdout);
#endif
            } else if (accum && pn1 != pn2) {
                while (pn1->pn_next != pn2) {
                    pn1 = RecycleTree(pn1, tc);
                    --pn->pn_count;
                }
                pn1->setKind(TOK_XMLTEXT);
                pn1->setOp(JSOP_STRING);
                pn1->setArity(PN_NULLARY);
                pn1->pn_atom = js_AtomizeString(cx, accum);
                if (!pn1->pn_atom)
                    return JS_FALSE;
                JS_ASSERT(pnp != &pn1->pn_next);
                *pnp = pn1;
            }
            pnp = &pn2->pn_next;
            pn1 = *pnp;
            accum = NULL;
            continue;
        }

        if (accum) {
            {
                AutoStringRooter tvr(cx, accum);
                str = ((tt == TOK_XMLSTAGO || tt == TOK_XMLPTAGC) && i != 0)
                      ? js_AddAttributePart(cx, i & 1, accum, str)
                      : js_ConcatStrings(cx, accum, str);
            }
            if (!str)
                return JS_FALSE;
#ifdef DEBUG_brendanXXX
            printf("2: %d, %d => ", i, j);
            FileEscapedString(stdout, str, 0);
            printf(" (%u)\n", str->length());
#endif
            ++j;
        }
        accum = str;
    }

    if (accum) {
        str = NULL;
        if ((pn->pn_xflags & PNX_CANTFOLD) == 0) {
            if (tt == TOK_XMLPTAGC)
                str = cx->runtime->atomState.ptagcAtom;
            else if (tt == TOK_XMLSTAGO || tt == TOK_XMLETAGO)
                str = cx->runtime->atomState.tagcAtom;
        }
        if (str) {
            accum = js_ConcatStrings(cx, accum, str);
            if (!accum)
                return JS_FALSE;
        }

        JS_ASSERT(*pnp == pn1);
        while (pn1->pn_next) {
            pn1 = RecycleTree(pn1, tc);
            --pn->pn_count;
        }
        pn1->setKind(TOK_XMLTEXT);
        pn1->setOp(JSOP_STRING);
        pn1->setArity(PN_NULLARY);
        pn1->pn_atom = js_AtomizeString(cx, accum);
        if (!pn1->pn_atom)
            return JS_FALSE;
        JS_ASSERT(pnp != &pn1->pn_next);
        *pnp = pn1;
    }

    if (pn1 && pn->pn_count == 1) {
        /*
         * Only one node under pn, and it has been folded: move pn1 onto pn
         * unless pn is an XML root (in which case we need it to tell the code
         * generator to emit a JSOP_TOXML or JSOP_TOXMLLIST op).  If pn is an
         * XML root *and* it's a point-tag, rewrite it to TOK_XMLELEM to avoid
         * extra "<" and "/>" bracketing at runtime.
         */
        if (!(pn->pn_xflags & PNX_XMLROOT)) {
            pn->become(pn1);
        } else if (tt == TOK_XMLPTAGC) {
            pn->setKind(TOK_XMLELEM);
            pn->setOp(JSOP_TOXML);
        }
    }
    return JS_TRUE;
}

#endif /* JS_HAS_XML_SUPPORT */

enum Truthiness { Truthy, Falsy, Unknown };

static Truthiness
Boolish(JSParseNode *pn)
{
    switch (pn->getOp()) {
      case JSOP_DOUBLE:
        return (pn->pn_dval != 0 && !JSDOUBLE_IS_NaN(pn->pn_dval)) ? Truthy : Falsy;

      case JSOP_STRING:
        return (pn->pn_atom->length() > 0) ? Truthy : Falsy;

#if JS_HAS_GENERATOR_EXPRS
      case JSOP_CALL:
      {
        /*
         * A generator expression as an if or loop condition has no effects, it
         * simply results in a truthy object reference. This condition folding
         * is needed for the decompiler. See bug 442342 and bug 443074.
         */
        if (pn->pn_count != 1)
            return Unknown;
        JSParseNode *pn2 = pn->pn_head;
        if (!pn2->isKind(TOK_FUNCTION))
            return Unknown;
        if (!(pn2->pn_funbox->tcflags & TCF_GENEXP_LAMBDA))
            return Unknown;
        return Truthy;
      }
#endif

      case JSOP_DEFFUN:
      case JSOP_LAMBDA:
      case JSOP_TRUE:
        return Truthy;

      case JSOP_NULL:
      case JSOP_FALSE:
        return Falsy;

      default:
        return Unknown;
    }
}

JSBool
js_FoldConstants(JSContext *cx, JSParseNode *pn, JSTreeContext *tc, bool inCond)
{
    JSParseNode *pn1 = NULL, *pn2 = NULL, *pn3 = NULL;

    JS_CHECK_RECURSION(cx, return JS_FALSE);

    switch (pn->getArity()) {
      case PN_FUNC:
      {
        uint32 oldflags = tc->flags;
        JSFunctionBox *oldlist = tc->functionList;

        tc->flags = pn->pn_funbox->tcflags;
        tc->functionList = pn->pn_funbox->kids;
        if (!js_FoldConstants(cx, pn->pn_body, tc))
            return JS_FALSE;
        pn->pn_funbox->kids = tc->functionList;
        tc->flags = oldflags;
        tc->functionList = oldlist;
        break;
      }

      case PN_LIST:
      {
        /* Propagate inCond through logical connectives. */
        bool cond = inCond && (pn->isKind(TOK_OR) || pn->isKind(TOK_AND));

        /* Don't fold a parenthesized call expression. See bug 537673. */
        pn1 = pn2 = pn->pn_head;
        if ((pn->isKind(TOK_LP) || pn->isKind(TOK_NEW)) && pn2->isInParens())
            pn2 = pn2->pn_next;

        /* Save the list head in pn1 for later use. */
        for (; pn2; pn2 = pn2->pn_next) {
            if (!js_FoldConstants(cx, pn2, tc, cond))
                return JS_FALSE;
        }
        break;
      }

      case PN_TERNARY:
        /* Any kid may be null (e.g. for (;;)). */
        pn1 = pn->pn_kid1;
        pn2 = pn->pn_kid2;
        pn3 = pn->pn_kid3;
        if (pn1 && !js_FoldConstants(cx, pn1, tc, pn->isKind(TOK_IF)))
            return JS_FALSE;
        if (pn2) {
            if (!js_FoldConstants(cx, pn2, tc, pn->isKind(TOK_FORHEAD)))
                return JS_FALSE;
            if (pn->isKind(TOK_FORHEAD) && pn2->isOp(JSOP_TRUE)) {
                RecycleTree(pn2, tc);
                pn->pn_kid2 = NULL;
            }
        }
        if (pn3 && !js_FoldConstants(cx, pn3, tc))
            return JS_FALSE;
        break;

      case PN_BINARY:
        pn1 = pn->pn_left;
        pn2 = pn->pn_right;

        /* Propagate inCond through logical connectives. */
        if (pn->isKind(TOK_OR) || pn->isKind(TOK_AND)) {
            if (!js_FoldConstants(cx, pn1, tc, inCond))
                return JS_FALSE;
            if (!js_FoldConstants(cx, pn2, tc, inCond))
                return JS_FALSE;
            break;
        }

        /* First kid may be null (for default case in switch). */
        if (pn1 && !js_FoldConstants(cx, pn1, tc, pn->isKind(TOK_WHILE)))
            return JS_FALSE;
        if (!js_FoldConstants(cx, pn2, tc, pn->isKind(TOK_DO)))
            return JS_FALSE;
        break;

      case PN_UNARY:
        pn1 = pn->pn_kid;

        /*
         * Kludge to deal with typeof expressions: because constant folding
         * can turn an expression into a name node, we have to check here,
         * before folding, to see if we should throw undefined name errors.
         *
         * NB: We know that if pn->pn_op is JSOP_TYPEOF, pn1 will not be
         * null. This assumption does not hold true for other unary
         * expressions.
         */
        if (pn->isOp(JSOP_TYPEOF) && !pn1->isKind(TOK_NAME))
            pn->setOp(JSOP_TYPEOFEXPR);

        if (pn1 && !js_FoldConstants(cx, pn1, tc, pn->isOp(JSOP_NOT)))
            return JS_FALSE;
        break;

      case PN_NAME:
        /*
         * Skip pn1 down along a chain of dotted member expressions to avoid
         * excessive recursion.  Our only goal here is to fold constants (if
         * any) in the primary expression operand to the left of the first
         * dot in the chain.
         */
        if (!pn->isUsed()) {
            pn1 = pn->pn_expr;
            while (pn1 && pn1->isArity(PN_NAME) && !pn1->isUsed())
                pn1 = pn1->pn_expr;
            if (pn1 && !js_FoldConstants(cx, pn1, tc))
                return JS_FALSE;
        }
        break;

      case PN_NAMESET:
        pn1 = pn->pn_tree;
        if (!js_FoldConstants(cx, pn1, tc))
            return JS_FALSE;
        break;

      case PN_NULLARY:
        break;
    }

    switch (pn->getKind()) {
      case TOK_IF:
        if (ContainsStmt(pn2, TOK_VAR) || ContainsStmt(pn3, TOK_VAR))
            break;
        /* FALL THROUGH */

      case TOK_HOOK:
        /* Reduce 'if (C) T; else E' into T for true C, E for false. */
        switch (pn1->getKind()) {
          case TOK_NUMBER:
            if (pn1->pn_dval == 0 || JSDOUBLE_IS_NaN(pn1->pn_dval))
                pn2 = pn3;
            break;
          case TOK_STRING:
            if (pn1->pn_atom->length() == 0)
                pn2 = pn3;
            break;
          case TOK_PRIMARY:
            if (pn1->isOp(JSOP_TRUE))
                break;
            if (pn1->isOp(JSOP_FALSE) || pn1->isOp(JSOP_NULL)) {
                pn2 = pn3;
                break;
            }
            /* FALL THROUGH */
          default:
            /* Early return to dodge common code that copies pn2 to pn. */
            return JS_TRUE;
        }

#if JS_HAS_GENERATOR_EXPRS
        /* Don't fold a trailing |if (0)| in a generator expression. */
        if (!pn2 && (tc->flags & TCF_GENEXP_LAMBDA))
            break;
#endif

        if (pn2 && !pn2->isDefn())
            pn->become(pn2);
        if (!pn2 || (pn->isKind(TOK_SEMI) && !pn->pn_kid)) {
            /*
             * False condition and no else, or an empty then-statement was
             * moved up over pn.  Either way, make pn an empty block (not an
             * empty statement, which does not decompile, even when labeled).
             * NB: pn must be a TOK_IF as TOK_HOOK can never have a null kid
             * or an empty statement for a child.
             */
            pn->setKind(TOK_LC);
            pn->setArity(PN_LIST);
            pn->makeEmpty();
        }
        RecycleTree(pn2, tc);
        if (pn3 && pn3 != pn2)
            RecycleTree(pn3, tc);
        break;

      case TOK_OR:
      case TOK_AND:
        if (inCond) {
            if (pn->isArity(PN_LIST)) {
                JSParseNode **pnp = &pn->pn_head;
                JS_ASSERT(*pnp == pn1);
                do {
                    Truthiness t = Boolish(pn1);
                    if (t == Unknown) {
                        pnp = &pn1->pn_next;
                        continue;
                    }
                    if ((t == Truthy) == pn->isKind(TOK_OR)) {
                        for (pn2 = pn1->pn_next; pn2; pn2 = pn3) {
                            pn3 = pn2->pn_next;
                            RecycleTree(pn2, tc);
                            --pn->pn_count;
                        }
                        pn1->pn_next = NULL;
                        break;
                    }
                    JS_ASSERT((t == Truthy) == pn->isKind(TOK_AND));
                    if (pn->pn_count == 1)
                        break;
                    *pnp = pn1->pn_next;
                    RecycleTree(pn1, tc);
                    --pn->pn_count;
                } while ((pn1 = *pnp) != NULL);

                // We may have to change arity from LIST to BINARY.
                pn1 = pn->pn_head;
                if (pn->pn_count == 2) {
                    pn2 = pn1->pn_next;
                    pn1->pn_next = NULL;
                    JS_ASSERT(!pn2->pn_next);
                    pn->setArity(PN_BINARY);
                    pn->pn_left = pn1;
                    pn->pn_right = pn2;
                } else if (pn->pn_count == 1) {
                    pn->become(pn1);
                    RecycleTree(pn1, tc);
                }
            } else {
                Truthiness t = Boolish(pn1);
                if (t != Unknown) {
                    if ((t == Truthy) == pn->isKind(TOK_OR)) {
                        RecycleTree(pn2, tc);
                        pn->become(pn1);
                    } else {
                        JS_ASSERT((t == Truthy) == pn->isKind(TOK_AND));
                        RecycleTree(pn1, tc);
                        pn->become(pn2);
                    }
                }
            }
        }
        break;

      case TOK_ASSIGN:
        /*
         * Compound operators such as *= should be subject to folding, in case
         * the left-hand side is constant, and so that the decompiler produces
         * the same string that you get from decompiling a script or function
         * compiled from that same string.  As with +, += is special.
         */
        if (pn->isOp(JSOP_NOP))
            break;
        if (!pn->isOp(JSOP_ADD))
            goto do_binary_op;
        /* FALL THROUGH */

      case TOK_PLUS:
        if (pn->isArity(PN_LIST)) {
            /*
             * Any string literal term with all others number or string means
             * this is a concatenation.  If any term is not a string or number
             * literal, we can't fold.
             */
            JS_ASSERT(pn->pn_count > 2);
            if (pn->pn_xflags & PNX_CANTFOLD)
                return JS_TRUE;
            if (pn->pn_xflags != PNX_STRCAT)
                goto do_binary_op;

            /* Ok, we're concatenating: convert non-string constant operands. */
            size_t length = 0;
            for (pn2 = pn1; pn2; pn2 = pn2->pn_next) {
                if (!FoldType(cx, pn2, TOK_STRING))
                    return JS_FALSE;
                /* XXX fold only if all operands convert to string */
                if (!pn2->isKind(TOK_STRING))
                    return JS_TRUE;
                length += pn2->pn_atom->length();
            }

            /* Allocate a new buffer and string descriptor for the result. */
            jschar *chars = (jschar *) cx->malloc_((length + 1) * sizeof(jschar));
            if (!chars)
                return JS_FALSE;
            chars[length] = 0;
            JSString *str = js_NewString(cx, chars, length);
            if (!str) {
                cx->free_(chars);
                return JS_FALSE;
            }

            /* Fill the buffer, advancing chars and recycling kids as we go. */
            for (pn2 = pn1; pn2; pn2 = RecycleTree(pn2, tc)) {
                JSAtom *atom = pn2->pn_atom;
                size_t length2 = atom->length();
                js_strncpy(chars, atom->chars(), length2);
                chars += length2;
            }
            JS_ASSERT(*chars == 0);

            /* Atomize the result string and mutate pn to refer to it. */
            pn->pn_atom = js_AtomizeString(cx, str);
            if (!pn->pn_atom)
                return JS_FALSE;
            pn->setKind(TOK_STRING);
            pn->setOp(JSOP_STRING);
            pn->setArity(PN_NULLARY);
            break;
        }

        /* Handle a binary string concatenation. */
        JS_ASSERT(pn->isArity(PN_BINARY));
        if (pn1->isKind(TOK_STRING) || pn2->isKind(TOK_STRING)) {
            JSString *left, *right, *str;

            if (!FoldType(cx, !pn1->isKind(TOK_STRING) ? pn1 : pn2, TOK_STRING))
                return JS_FALSE;
            if (!pn1->isKind(TOK_STRING) || !pn2->isKind(TOK_STRING))
                return JS_TRUE;
            left = pn1->pn_atom;
            right = pn2->pn_atom;
            str = js_ConcatStrings(cx, left, right);
            if (!str)
                return JS_FALSE;
            pn->pn_atom = js_AtomizeString(cx, str);
            if (!pn->pn_atom)
                return JS_FALSE;
            pn->setKind(TOK_STRING);
            pn->setOp(JSOP_STRING);
            pn->setArity(PN_NULLARY);
            RecycleTree(pn1, tc);
            RecycleTree(pn2, tc);
            break;
        }

        /* Can't concatenate string literals, let's try numbers. */
        goto do_binary_op;

      case TOK_STAR:
      case TOK_SHOP:
      case TOK_MINUS:
      case TOK_DIVOP:
      do_binary_op:
        if (pn->isArity(PN_LIST)) {
            JS_ASSERT(pn->pn_count > 2);
            for (pn2 = pn1; pn2; pn2 = pn2->pn_next) {
                if (!FoldType(cx, pn2, TOK_NUMBER))
                    return JS_FALSE;
            }
            for (pn2 = pn1; pn2; pn2 = pn2->pn_next) {
                /* XXX fold only if all operands convert to number */
                if (!pn2->isKind(TOK_NUMBER))
                    break;
            }
            if (!pn2) {
                JSOp op = pn->getOp();

                pn2 = pn1->pn_next;
                pn3 = pn2->pn_next;
                if (!FoldBinaryNumeric(cx, op, pn1, pn2, pn, tc))
                    return JS_FALSE;
                while ((pn2 = pn3) != NULL) {
                    pn3 = pn2->pn_next;
                    if (!FoldBinaryNumeric(cx, op, pn, pn2, pn, tc))
                        return JS_FALSE;
                }
            }
        } else {
            JS_ASSERT(pn->isArity(PN_BINARY));
            if (!FoldType(cx, pn1, TOK_NUMBER) ||
                !FoldType(cx, pn2, TOK_NUMBER)) {
                return JS_FALSE;
            }
            if (pn1->isKind(TOK_NUMBER) && pn2->isKind(TOK_NUMBER)) {
                if (!FoldBinaryNumeric(cx, pn->getOp(), pn1, pn2, pn, tc))
                    return JS_FALSE;
            }
        }
        break;

      case TOK_UNARYOP:
        if (pn1->isKind(TOK_NUMBER)) {
            jsdouble d;

            /* Operate on one numeric constant. */
            d = pn1->pn_dval;
            switch (pn->getOp()) {
              case JSOP_BITNOT:
                d = ~js_DoubleToECMAInt32(d);
                break;

              case JSOP_NEG:
                d = -d;
                break;

              case JSOP_POS:
                break;

              case JSOP_NOT:
                pn->setKind(TOK_PRIMARY);
                pn->setOp((d == 0 || JSDOUBLE_IS_NaN(d)) ? JSOP_TRUE : JSOP_FALSE);
                pn->setArity(PN_NULLARY);
                /* FALL THROUGH */

              default:
                /* Return early to dodge the common TOK_NUMBER code. */
                return JS_TRUE;
            }
            pn->setKind(TOK_NUMBER);
            pn->setOp(JSOP_DOUBLE);
            pn->setArity(PN_NULLARY);
            pn->pn_dval = d;
            RecycleTree(pn1, tc);
        } else if (pn1->isKind(TOK_PRIMARY)) {
            if (pn->isOp(JSOP_NOT) && (pn1->isOp(JSOP_TRUE) || pn1->isOp(JSOP_FALSE))) {
                pn->become(pn1);
                pn->setOp(pn->isOp(JSOP_TRUE) ? JSOP_FALSE : JSOP_TRUE);
                RecycleTree(pn1, tc);
            }
        }
        break;

#if JS_HAS_XML_SUPPORT
      case TOK_XMLELEM:
      case TOK_XMLLIST:
      case TOK_XMLPTAGC:
      case TOK_XMLSTAGO:
      case TOK_XMLETAGO:
      case TOK_XMLNAME:
        if (pn->isArity(PN_LIST)) {
            JS_ASSERT(pn->isKind(TOK_XMLLIST) || pn->pn_count != 0);
            if (!FoldXMLConstants(cx, pn, tc))
                return JS_FALSE;
        }
        break;

      case TOK_AT:
        if (pn1->isKind(TOK_XMLNAME)) {
            JSObjectBox *xmlbox;

            Value v = StringValue(pn1->pn_atom);
            if (!js_ToAttributeName(cx, &v))
                return JS_FALSE;
            JS_ASSERT(v.isObject());

            xmlbox = tc->parser->newObjectBox(&v.toObject());
            if (!xmlbox)
                return JS_FALSE;

            pn->setKind(TOK_XMLNAME);
            pn->setOp(JSOP_OBJECT);
            pn->setArity(PN_NULLARY);
            pn->pn_objbox = xmlbox;
            RecycleTree(pn1, tc);
        }
        break;
#endif /* JS_HAS_XML_SUPPORT */

      default:;
    }

    if (inCond) {
        Truthiness t = Boolish(pn);
        if (t != Unknown) {
            /*
             * We can turn function nodes into constant nodes here, but mutating function
             * nodes is tricky --- in particular, mutating a function node that appears on
             * a method list corrupts the method list. However, methods are M's in
             * statements of the form 'this.foo = M;', which we never fold, so we're okay.
             */
            PrepareNodeForMutation(pn, tc);
            pn->setKind(TOK_PRIMARY);
            pn->setOp(t == Truthy ? JSOP_TRUE : JSOP_FALSE);
            pn->setArity(PN_NULLARY);
        }
    }

    return JS_TRUE;
}
