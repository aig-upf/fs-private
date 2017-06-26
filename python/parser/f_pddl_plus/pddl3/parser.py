# Generated from java-escape by ANTLR 4.5
# encoding: utf-8
from antlr4 import *
from io import StringIO
package = globals().get("__package__", None)
ischild = len(package)>0 if package is not None else False
if ischild:
    from .Pddl3Listener import Pddl3Listener
    from .Pddl3Visitor import Pddl3Visitor
else:
    from Pddl3Listener import Pddl3Listener
    from Pddl3Visitor import Pddl3Visitor

def serializedATN():
    with StringIO() as buf:
        buf.write("\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\3z")
        buf.write("\u03d8\4\2\t\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7")
        buf.write("\4\b\t\b\4\t\t\t\4\n\t\n\4\13\t\13\4\f\t\f\4\r\t\r\4\16")
        buf.write("\t\16\4\17\t\17\4\20\t\20\4\21\t\21\4\22\t\22\4\23\t\23")
        buf.write("\4\24\t\24\4\25\t\25\4\26\t\26\4\27\t\27\4\30\t\30\4\31")
        buf.write("\t\31\4\32\t\32\4\33\t\33\4\34\t\34\4\35\t\35\4\36\t\36")
        buf.write("\4\37\t\37\4 \t \4!\t!\4\"\t\"\4#\t#\4$\t$\4%\t%\4&\t")
        buf.write("&\4\'\t\'\4(\t(\4)\t)\4*\t*\4+\t+\4,\t,\4-\t-\4.\t.\4")
        buf.write("/\t/\4\60\t\60\4\61\t\61\4\62\t\62\4\63\t\63\4\64\t\64")
        buf.write("\4\65\t\65\4\66\t\66\4\67\t\67\48\t8\49\t9\4:\t:\4;\t")
        buf.write(";\4<\t<\4=\t=\4>\t>\4?\t?\4@\t@\4A\tA\4B\tB\4C\tC\4D\t")
        buf.write("D\4E\tE\4F\tF\4G\tG\4H\tH\4I\tI\4J\tJ\4K\tK\4L\tL\3\2")
        buf.write("\3\2\5\2\u009b\n\2\3\3\3\3\3\3\3\3\5\3\u00a1\n\3\3\3\5")
        buf.write("\3\u00a4\n\3\3\3\5\3\u00a7\n\3\3\3\5\3\u00aa\n\3\3\3\5")
        buf.write("\3\u00ad\n\3\3\3\5\3\u00b0\n\3\3\3\5\3\u00b3\n\3\3\3\7")
        buf.write("\3\u00b6\n\3\f\3\16\3\u00b9\13\3\3\3\3\3\3\4\3\4\3\4\3")
        buf.write("\4\3\4\3\5\3\5\3\5\3\5\3\5\3\6\3\6\3\6\6\6\u00ca\n\6\r")
        buf.write("\6\16\6\u00cb\3\6\3\6\3\7\3\7\3\7\3\7\3\7\3\b\7\b\u00d6")
        buf.write("\n\b\f\b\16\b\u00d9\13\b\3\b\6\b\u00dc\n\b\r\b\16\b\u00dd")
        buf.write("\3\b\7\b\u00e1\n\b\f\b\16\b\u00e4\13\b\5\b\u00e6\n\b\3")
        buf.write("\t\6\t\u00e9\n\t\r\t\16\t\u00ea\3\t\3\t\3\t\3\n\3\n\3")
        buf.write("\n\6\n\u00f3\n\n\r\n\16\n\u00f4\3\n\3\n\3\n\5\n\u00fa")
        buf.write("\n\n\3\13\3\13\3\f\3\f\3\f\3\f\3\f\3\r\6\r\u0104\n\r\r")
        buf.write("\r\16\r\u0105\3\r\3\r\5\r\u010a\n\r\7\r\u010c\n\r\f\r")
        buf.write("\16\r\u010f\13\r\3\16\3\16\3\16\3\16\3\16\3\17\3\17\3")
        buf.write("\20\3\20\3\21\3\21\3\21\3\21\3\21\3\22\3\22\3\22\6\22")
        buf.write("\u0122\n\22\r\22\16\22\u0123\3\22\3\22\3\23\3\23\3\23")
        buf.write("\3\23\3\23\3\24\3\24\3\25\7\25\u0130\n\25\f\25\16\25\u0133")
        buf.write("\13\25\3\25\6\25\u0136\n\25\r\25\16\25\u0137\3\25\7\25")
        buf.write("\u013b\n\25\f\25\16\25\u013e\13\25\5\25\u0140\n\25\3\26")
        buf.write("\6\26\u0143\n\26\r\26\16\26\u0144\3\26\3\26\3\26\3\27")
        buf.write("\3\27\3\27\3\27\3\27\3\30\3\30\3\30\3\30\5\30\u0153\n")
        buf.write("\30\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31\3\31")
        buf.write("\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\32\3\33")
        buf.write("\3\33\3\34\3\34\3\35\3\35\3\35\3\35\5\35\u0171\n\35\5")
        buf.write("\35\u0173\n\35\3\35\3\35\3\35\3\35\5\35\u0179\n\35\5\35")
        buf.write("\u017b\n\35\3\36\3\36\3\36\3\36\5\36\u0181\n\36\5\36\u0183")
        buf.write("\n\36\3\37\3\37\3\37\3\37\7\37\u0189\n\37\f\37\16\37\u018c")
        buf.write("\13\37\3\37\3\37\3\37\3\37\7\37\u0192\n\37\f\37\16\37")
        buf.write("\u0195\13\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37\3")
        buf.write("\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37")
        buf.write("\3\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37\3\37")
        buf.write("\5\37\u01b5\n\37\3 \3 \3 \3 \3 \3 \3!\3!\3!\3!\3!\3!\3")
        buf.write("\"\3\"\3\"\7\"\u01c6\n\"\f\"\16\"\u01c9\13\"\3\"\3\"\3")
        buf.write("#\3#\3$\3$\3$\3$\3$\3$\3$\3$\3$\3$\3$\3$\3$\3$\3$\3$\3")
        buf.write("$\3$\3$\5$\u01e2\n$\3%\3%\3%\3%\3%\3%\5%\u01ea\n%\3%\3")
        buf.write("%\3%\3%\5%\u01f0\n%\5%\u01f2\n%\3&\3&\3&\3&\7&\u01f8\n")
        buf.write("&\f&\16&\u01fb\13&\3&\3&\3&\3&\3&\3&\3&\3&\3&\5&\u0206")
        buf.write("\n&\3\'\3\'\3\'\3\'\5\'\u020c\n\'\3\'\3\'\3\'\5\'\u0211")
        buf.write("\n\'\3(\3(\3(\3(\3(\3(\3(\3(\3(\3(\3(\3(\5(\u021f\n(\3")
        buf.write(")\3)\3*\3*\3+\3+\3+\3+\3+\3+\3,\3,\3,\3,\3,\3,\3,\3,\3")
        buf.write(",\3,\3,\3,\3,\5,\u0238\n,\3-\3-\3-\7-\u023d\n-\f-\16-")
        buf.write("\u0240\13-\3-\3-\3-\5-\u0245\n-\3.\3.\3.\7.\u024a\n.\f")
        buf.write(".\16.\u024d\13.\3.\3.\5.\u0251\n.\3/\3/\3/\3/\3/\3/\3")
        buf.write("/\3/\3/\3/\3/\3/\3/\3/\3/\5/\u0262\n/\3\60\3\60\3\60\3")
        buf.write("\60\3\60\3\60\3\60\3\60\3\60\3\60\3\60\3\60\5\60\u0270")
        buf.write("\n\60\3\61\3\61\3\61\7\61\u0275\n\61\f\61\16\61\u0278")
        buf.write("\13\61\3\61\3\61\5\61\u027c\n\61\3\62\3\62\3\63\3\63\3")
        buf.write("\64\3\64\3\65\3\65\3\66\3\66\3\66\6\66\u0289\n\66\r\66")
        buf.write("\16\66\u028a\3\66\3\66\3\66\3\66\3\66\5\66\u0292\n\66")
        buf.write("\3\67\3\67\3\67\3\67\3\67\3\67\3\67\3\67\3\67\3\67\3\67")
        buf.write("\3\67\5\67\u02a0\n\67\38\38\39\39\59\u02a6\n9\3:\3:\3")
        buf.write(":\7:\u02ab\n:\f:\16:\u02ae\13:\3:\3:\3:\3:\3:\3:\3:\3")
        buf.write(":\3:\3:\3:\3:\3:\3:\3:\3:\3:\3:\3:\3:\3:\3:\5:\u02c6\n")
        buf.write(":\3;\3;\3;\3;\3;\3;\3;\3;\3;\3;\3;\3;\3;\3;\3;\3;\3;\3")
        buf.write(";\5;\u02da\n;\3<\3<\3<\3<\3<\3<\3=\3=\3=\3=\3=\3=\3=\5")
        buf.write("=\u02e9\n=\3=\3=\3=\3=\5=\u02ef\n=\3>\3>\3>\3>\3>\5>\u02f6")
        buf.write("\n>\3>\5>\u02f9\n>\3>\3>\3>\5>\u02fe\n>\3>\5>\u0301\n")
        buf.write(">\3>\3>\3?\3?\3?\3?\3?\3@\3@\3@\3@\3@\3A\3A\3A\3A\3A\3")
        buf.write("B\3B\3B\7B\u0317\nB\fB\16B\u031a\13B\3B\3B\3C\3C\3C\3")
        buf.write("C\3C\3C\3C\3C\3C\3C\3C\3C\3C\5C\u032b\nC\3D\3D\3D\3D\3")
        buf.write("D\3D\5D\u0333\nD\3E\3E\3E\7E\u0338\nE\fE\16E\u033b\13")
        buf.write("E\3E\3E\3F\3F\3F\3F\3F\3G\3G\3G\3G\3G\3H\3H\3H\7H\u034c")
        buf.write("\nH\fH\16H\u034f\13H\3H\3H\3H\3H\3H\3H\3H\3H\3H\3H\3H")
        buf.write("\3H\5H\u035d\nH\3H\3H\3H\3H\5H\u0363\nH\3I\3I\3I\3I\3")
        buf.write("I\3I\3J\3J\3K\3K\3K\3K\3K\3K\3K\3K\3K\3K\6K\u0377\nK\r")
        buf.write("K\16K\u0378\3K\3K\3K\3K\3K\3K\3K\3K\3K\3K\3K\3K\3K\5K")
        buf.write("\u0388\nK\3L\3L\3L\7L\u038d\nL\fL\16L\u0390\13L\3L\3L")
        buf.write("\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3")
        buf.write("L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3")
        buf.write("L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3")
        buf.write("L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\3L\5L\u03d6\nL\3L\2")
        buf.write("\2M\2\4\6\b\n\f\16\20\22\24\26\30\32\34\36 \"$&(*,.\60")
        buf.write("\62\64\668:<>@BDFHJLNPRTVXZ\\^`bdfhjlnprtvxz|~\u0080\u0082")
        buf.write("\u0084\u0086\u0088\u008a\u008c\u008e\u0090\u0092\u0094")
        buf.write("\u0096\2\n\3\2HI\3\2#$\4\2\n\n(*\3\2()\4\2\35\35+.\3\2")
        buf.write("/\63\4\2\35\35-.\3\2;<\u0401\2\u009a\3\2\2\2\4\u009c\3")
        buf.write("\2\2\2\6\u00bc\3\2\2\2\b\u00c1\3\2\2\2\n\u00c6\3\2\2\2")
        buf.write("\f\u00cf\3\2\2\2\16\u00e5\3\2\2\2\20\u00e8\3\2\2\2\22")
        buf.write("\u00f9\3\2\2\2\24\u00fb\3\2\2\2\26\u00fd\3\2\2\2\30\u010d")
        buf.write("\3\2\2\2\32\u0110\3\2\2\2\34\u0115\3\2\2\2\36\u0117\3")
        buf.write("\2\2\2 \u0119\3\2\2\2\"\u011e\3\2\2\2$\u0127\3\2\2\2&")
        buf.write("\u012c\3\2\2\2(\u013f\3\2\2\2*\u0142\3\2\2\2,\u0149\3")
        buf.write("\2\2\2.\u0152\3\2\2\2\60\u0154\3\2\2\2\62\u015e\3\2\2")
        buf.write("\2\64\u0168\3\2\2\2\66\u016a\3\2\2\28\u0172\3\2\2\2:\u0182")
        buf.write("\3\2\2\2<\u01b4\3\2\2\2>\u01b6\3\2\2\2@\u01bc\3\2\2\2")
        buf.write("B\u01c2\3\2\2\2D\u01cc\3\2\2\2F\u01e1\3\2\2\2H\u01f1\3")
        buf.write("\2\2\2J\u0205\3\2\2\2L\u0210\3\2\2\2N\u021e\3\2\2\2P\u0220")
        buf.write("\3\2\2\2R\u0222\3\2\2\2T\u0224\3\2\2\2V\u0237\3\2\2\2")
        buf.write("X\u0244\3\2\2\2Z\u0250\3\2\2\2\\\u0261\3\2\2\2^\u026f")
        buf.write("\3\2\2\2`\u027b\3\2\2\2b\u027d\3\2\2\2d\u027f\3\2\2\2")
        buf.write("f\u0281\3\2\2\2h\u0283\3\2\2\2j\u0291\3\2\2\2l\u029f\3")
        buf.write("\2\2\2n\u02a1\3\2\2\2p\u02a5\3\2\2\2r\u02c5\3\2\2\2t\u02d9")
        buf.write("\3\2\2\2v\u02db\3\2\2\2x\u02ee\3\2\2\2z\u02f0\3\2\2\2")
        buf.write("|\u0304\3\2\2\2~\u0309\3\2\2\2\u0080\u030e\3\2\2\2\u0082")
        buf.write("\u0313\3\2\2\2\u0084\u032a\3\2\2\2\u0086\u0332\3\2\2\2")
        buf.write("\u0088\u0334\3\2\2\2\u008a\u033e\3\2\2\2\u008c\u0343\3")
        buf.write("\2\2\2\u008e\u0362\3\2\2\2\u0090\u0364\3\2\2\2\u0092\u036a")
        buf.write("\3\2\2\2\u0094\u0387\3\2\2\2\u0096\u03d5\3\2\2\2\u0098")
        buf.write("\u009b\5\4\3\2\u0099\u009b\5z>\2\u009a\u0098\3\2\2\2\u009a")
        buf.write("\u0099\3\2\2\2\u009b\3\3\2\2\2\u009c\u009d\7\3\2\2\u009d")
        buf.write("\u009e\7\4\2\2\u009e\u00a0\5\b\5\2\u009f\u00a1\5\n\6\2")
        buf.write("\u00a0\u009f\3\2\2\2\u00a0\u00a1\3\2\2\2\u00a1\u00a3\3")
        buf.write("\2\2\2\u00a2\u00a4\5\f\7\2\u00a3\u00a2\3\2\2\2\u00a3\u00a4")
        buf.write("\3\2\2\2\u00a4\u00a6\3\2\2\2\u00a5\u00a7\5 \21\2\u00a6")
        buf.write("\u00a5\3\2\2\2\u00a6\u00a7\3\2\2\2\u00a7\u00a9\3\2\2\2")
        buf.write("\u00a8\u00aa\5\"\22\2\u00a9\u00a8\3\2\2\2\u00a9\u00aa")
        buf.write("\3\2\2\2\u00aa\u00ac\3\2\2\2\u00ab\u00ad\5\26\f\2\u00ac")
        buf.write("\u00ab\3\2\2\2\u00ac\u00ad\3\2\2\2\u00ad\u00af\3\2\2\2")
        buf.write("\u00ae\u00b0\5\6\4\2\u00af\u00ae\3\2\2\2\u00af\u00b0\3")
        buf.write("\2\2\2\u00b0\u00b2\3\2\2\2\u00b1\u00b3\5,\27\2\u00b2\u00b1")
        buf.write("\3\2\2\2\u00b2\u00b3\3\2\2\2\u00b3\u00b7\3\2\2\2\u00b4")
        buf.write("\u00b6\5.\30\2\u00b5\u00b4\3\2\2\2\u00b6\u00b9\3\2\2\2")
        buf.write("\u00b7\u00b5\3\2\2\2\u00b7\u00b8\3\2\2\2\u00b8\u00ba\3")
        buf.write("\2\2\2\u00b9\u00b7\3\2\2\2\u00ba\u00bb\7\5\2\2\u00bb\5")
        buf.write("\3\2\2\2\u00bc\u00bd\7\3\2\2\u00bd\u00be\7\6\2\2\u00be")
        buf.write("\u00bf\5\30\r\2\u00bf\u00c0\7\5\2\2\u00c0\7\3\2\2\2\u00c1")
        buf.write("\u00c2\7\3\2\2\u00c2\u00c3\7\7\2\2\u00c3\u00c4\7H\2\2")
        buf.write("\u00c4\u00c5\7\5\2\2\u00c5\t\3\2\2\2\u00c6\u00c7\7\3\2")
        buf.write("\2\u00c7\u00c9\7\b\2\2\u00c8\u00ca\7G\2\2\u00c9\u00c8")
        buf.write("\3\2\2\2\u00ca\u00cb\3\2\2\2\u00cb\u00c9\3\2\2\2\u00cb")
        buf.write("\u00cc\3\2\2\2\u00cc\u00cd\3\2\2\2\u00cd\u00ce\7\5\2\2")
        buf.write("\u00ce\13\3\2\2\2\u00cf\u00d0\7\3\2\2\u00d0\u00d1\7\t")
        buf.write("\2\2\u00d1\u00d2\5\16\b\2\u00d2\u00d3\7\5\2\2\u00d3\r")
        buf.write("\3\2\2\2\u00d4\u00d6\7H\2\2\u00d5\u00d4\3\2\2\2\u00d6")
        buf.write("\u00d9\3\2\2\2\u00d7\u00d5\3\2\2\2\u00d7\u00d8\3\2\2\2")
        buf.write("\u00d8\u00e6\3\2\2\2\u00d9\u00d7\3\2\2\2\u00da\u00dc\5")
        buf.write("\20\t\2\u00db\u00da\3\2\2\2\u00dc\u00dd\3\2\2\2\u00dd")
        buf.write("\u00db\3\2\2\2\u00dd\u00de\3\2\2\2\u00de\u00e2\3\2\2\2")
        buf.write("\u00df\u00e1\7H\2\2\u00e0\u00df\3\2\2\2\u00e1\u00e4\3")
        buf.write("\2\2\2\u00e2\u00e0\3\2\2\2\u00e2\u00e3\3\2\2\2\u00e3\u00e6")
        buf.write("\3\2\2\2\u00e4\u00e2\3\2\2\2\u00e5\u00d7\3\2\2\2\u00e5")
        buf.write("\u00db\3\2\2\2\u00e6\17\3\2\2\2\u00e7\u00e9\7H\2\2\u00e8")
        buf.write("\u00e7\3\2\2\2\u00e9\u00ea\3\2\2\2\u00ea\u00e8\3\2\2\2")
        buf.write("\u00ea\u00eb\3\2\2\2\u00eb\u00ec\3\2\2\2\u00ec\u00ed\7")
        buf.write("\n\2\2\u00ed\u00ee\5\22\n\2\u00ee\21\3\2\2\2\u00ef\u00f0")
        buf.write("\7\3\2\2\u00f0\u00f2\7\13\2\2\u00f1\u00f3\5\24\13\2\u00f2")
        buf.write("\u00f1\3\2\2\2\u00f3\u00f4\3\2\2\2\u00f4\u00f2\3\2\2\2")
        buf.write("\u00f4\u00f5\3\2\2\2\u00f5\u00f6\3\2\2\2\u00f6\u00f7\7")
        buf.write("\5\2\2\u00f7\u00fa\3\2\2\2\u00f8\u00fa\5\24\13\2\u00f9")
        buf.write("\u00ef\3\2\2\2\u00f9\u00f8\3\2\2\2\u00fa\23\3\2\2\2\u00fb")
        buf.write("\u00fc\7H\2\2\u00fc\25\3\2\2\2\u00fd\u00fe\7\3\2\2\u00fe")
        buf.write("\u00ff\7\f\2\2\u00ff\u0100\5\30\r\2\u0100\u0101\7\5\2")
        buf.write("\2\u0101\27\3\2\2\2\u0102\u0104\5\32\16\2\u0103\u0102")
        buf.write("\3\2\2\2\u0104\u0105\3\2\2\2\u0105\u0103\3\2\2\2\u0105")
        buf.write("\u0106\3\2\2\2\u0106\u0109\3\2\2\2\u0107\u0108\7\n\2\2")
        buf.write("\u0108\u010a\5\36\20\2\u0109\u0107\3\2\2\2\u0109\u010a")
        buf.write("\3\2\2\2\u010a\u010c\3\2\2\2\u010b\u0103\3\2\2\2\u010c")
        buf.write("\u010f\3\2\2\2\u010d\u010b\3\2\2\2\u010d\u010e\3\2\2\2")
        buf.write("\u010e\31\3\2\2\2\u010f\u010d\3\2\2\2\u0110\u0111\7\3")
        buf.write("\2\2\u0111\u0112\5\34\17\2\u0112\u0113\5(\25\2\u0113\u0114")
        buf.write("\7\5\2\2\u0114\33\3\2\2\2\u0115\u0116\7H\2\2\u0116\35")
        buf.write("\3\2\2\2\u0117\u0118\7\r\2\2\u0118\37\3\2\2\2\u0119\u011a")
        buf.write("\7\3\2\2\u011a\u011b\7\16\2\2\u011b\u011c\5\16\b\2\u011c")
        buf.write("\u011d\7\5\2\2\u011d!\3\2\2\2\u011e\u011f\7\3\2\2\u011f")
        buf.write("\u0121\7\17\2\2\u0120\u0122\5$\23\2\u0121\u0120\3\2\2")
        buf.write("\2\u0122\u0123\3\2\2\2\u0123\u0121\3\2\2\2\u0123\u0124")
        buf.write("\3\2\2\2\u0124\u0125\3\2\2\2\u0125\u0126\7\5\2\2\u0126")
        buf.write("#\3\2\2\2\u0127\u0128\7\3\2\2\u0128\u0129\5&\24\2\u0129")
        buf.write("\u012a\5(\25\2\u012a\u012b\7\5\2\2\u012b%\3\2\2\2\u012c")
        buf.write("\u012d\7H\2\2\u012d\'\3\2\2\2\u012e\u0130\7I\2\2\u012f")
        buf.write("\u012e\3\2\2\2\u0130\u0133\3\2\2\2\u0131\u012f\3\2\2\2")
        buf.write("\u0131\u0132\3\2\2\2\u0132\u0140\3\2\2\2\u0133\u0131\3")
        buf.write("\2\2\2\u0134\u0136\5*\26\2\u0135\u0134\3\2\2\2\u0136\u0137")
        buf.write("\3\2\2\2\u0137\u0135\3\2\2\2\u0137\u0138\3\2\2\2\u0138")
        buf.write("\u013c\3\2\2\2\u0139\u013b\7I\2\2\u013a\u0139\3\2\2\2")
        buf.write("\u013b\u013e\3\2\2\2\u013c\u013a\3\2\2\2\u013c\u013d\3")
        buf.write("\2\2\2\u013d\u0140\3\2\2\2\u013e\u013c\3\2\2\2\u013f\u0131")
        buf.write("\3\2\2\2\u013f\u0135\3\2\2\2\u0140)\3\2\2\2\u0141\u0143")
        buf.write("\7I\2\2\u0142\u0141\3\2\2\2\u0143\u0144\3\2\2\2\u0144")
        buf.write("\u0142\3\2\2\2\u0144\u0145\3\2\2\2\u0145\u0146\3\2\2\2")
        buf.write("\u0146\u0147\7\n\2\2\u0147\u0148\5\22\n\2\u0148+\3\2\2")
        buf.write("\2\u0149\u014a\7\3\2\2\u014a\u014b\7\20\2\2\u014b\u014c")
        buf.write("\5\u0096L\2\u014c\u014d\7\5\2\2\u014d-\3\2\2\2\u014e\u0153")
        buf.write("\5\60\31\2\u014f\u0153\5F$\2\u0150\u0153\5T+\2\u0151\u0153")
        buf.write("\5\62\32\2\u0152\u014e\3\2\2\2\u0152\u014f\3\2\2\2\u0152")
        buf.write("\u0150\3\2\2\2\u0152\u0151\3\2\2\2\u0153/\3\2\2\2\u0154")
        buf.write("\u0155\7\3\2\2\u0155\u0156\7\21\2\2\u0156\u0157\5\64\33")
        buf.write("\2\u0157\u0158\7\22\2\2\u0158\u0159\7\3\2\2\u0159\u015a")
        buf.write("\5(\25\2\u015a\u015b\7\5\2\2\u015b\u015c\58\35\2\u015c")
        buf.write("\u015d\7\5\2\2\u015d\61\3\2\2\2\u015e\u015f\7\3\2\2\u015f")
        buf.write("\u0160\7\23\2\2\u0160\u0161\5\66\34\2\u0161\u0162\7\22")
        buf.write("\2\2\u0162\u0163\7\3\2\2\u0163\u0164\5(\25\2\u0164\u0165")
        buf.write("\7\5\2\2\u0165\u0166\5:\36\2\u0166\u0167\7\5\2\2\u0167")
        buf.write("\63\3\2\2\2\u0168\u0169\7H\2\2\u0169\65\3\2\2\2\u016a")
        buf.write("\u016b\7H\2\2\u016b\67\3\2\2\2\u016c\u0170\7\24\2\2\u016d")
        buf.write("\u016e\7\3\2\2\u016e\u0171\7\5\2\2\u016f\u0171\5<\37\2")
        buf.write("\u0170\u016d\3\2\2\2\u0170\u016f\3\2\2\2\u0171\u0173\3")
        buf.write("\2\2\2\u0172\u016c\3\2\2\2\u0172\u0173\3\2\2\2\u0173\u017a")
        buf.write("\3\2\2\2\u0174\u0178\7\25\2\2\u0175\u0176\7\3\2\2\u0176")
        buf.write("\u0179\7\5\2\2\u0177\u0179\5Z.\2\u0178\u0175\3\2\2\2\u0178")
        buf.write("\u0177\3\2\2\2\u0179\u017b\3\2\2\2\u017a\u0174\3\2\2\2")
        buf.write("\u017a\u017b\3\2\2\2\u017b9\3\2\2\2\u017c\u0180\7\26\2")
        buf.write("\2\u017d\u017e\7\3\2\2\u017e\u0181\7\5\2\2\u017f\u0181")
        buf.write("\5<\37\2\u0180\u017d\3\2\2\2\u0180\u017f\3\2\2\2\u0181")
        buf.write("\u0183\3\2\2\2\u0182\u017c\3\2\2\2\u0182\u0183\3\2\2\2")
        buf.write("\u0183;\3\2\2\2\u0184\u01b5\5B\"\2\u0185\u0186\7\3\2\2")
        buf.write("\u0186\u018a\7\27\2\2\u0187\u0189\5<\37\2\u0188\u0187")
        buf.write("\3\2\2\2\u0189\u018c\3\2\2\2\u018a\u0188\3\2\2\2\u018a")
        buf.write("\u018b\3\2\2\2\u018b\u018d\3\2\2\2\u018c\u018a\3\2\2\2")
        buf.write("\u018d\u01b5\7\5\2\2\u018e\u018f\7\3\2\2\u018f\u0193\7")
        buf.write("\30\2\2\u0190\u0192\5<\37\2\u0191\u0190\3\2\2\2\u0192")
        buf.write("\u0195\3\2\2\2\u0193\u0191\3\2\2\2\u0193\u0194\3\2\2\2")
        buf.write("\u0194\u0196\3\2\2\2\u0195\u0193\3\2\2\2\u0196\u01b5\7")
        buf.write("\5\2\2\u0197\u0198\7\3\2\2\u0198\u0199\7\31\2\2\u0199")
        buf.write("\u019a\5<\37\2\u019a\u019b\7\5\2\2\u019b\u01b5\3\2\2\2")
        buf.write("\u019c\u019d\7\3\2\2\u019d\u019e\7\32\2\2\u019e\u019f")
        buf.write("\5<\37\2\u019f\u01a0\5<\37\2\u01a0\u01a1\7\5\2\2\u01a1")
        buf.write("\u01b5\3\2\2\2\u01a2\u01a3\7\3\2\2\u01a3\u01a4\7\33\2")
        buf.write("\2\u01a4\u01a5\7\3\2\2\u01a5\u01a6\5(\25\2\u01a6\u01a7")
        buf.write("\7\5\2\2\u01a7\u01a8\5<\37\2\u01a8\u01a9\7\5\2\2\u01a9")
        buf.write("\u01b5\3\2\2\2\u01aa\u01ab\7\3\2\2\u01ab\u01ac\7\34\2")
        buf.write("\2\u01ac\u01ad\7\3\2\2\u01ad\u01ae\5(\25\2\u01ae\u01af")
        buf.write("\7\5\2\2\u01af\u01b0\5<\37\2\u01b0\u01b1\7\5\2\2\u01b1")
        buf.write("\u01b5\3\2\2\2\u01b2\u01b5\5@!\2\u01b3\u01b5\5> \2\u01b4")
        buf.write("\u0184\3\2\2\2\u01b4\u0185\3\2\2\2\u01b4\u018e\3\2\2\2")
        buf.write("\u01b4\u0197\3\2\2\2\u01b4\u019c\3\2\2\2\u01b4\u01a2\3")
        buf.write("\2\2\2\u01b4\u01aa\3\2\2\2\u01b4\u01b2\3\2\2\2\u01b4\u01b3")
        buf.write("\3\2\2\2\u01b5=\3\2\2\2\u01b6\u01b7\7\3\2\2\u01b7\u01b8")
        buf.write("\7\35\2\2\u01b8\u01b9\5D#\2\u01b9\u01ba\5D#\2\u01ba\u01bb")
        buf.write("\7\5\2\2\u01bb?\3\2\2\2\u01bc\u01bd\7\3\2\2\u01bd\u01be")
        buf.write("\5f\64\2\u01be\u01bf\5V,\2\u01bf\u01c0\5V,\2\u01c0\u01c1")
        buf.write("\7\5\2\2\u01c1A\3\2\2\2\u01c2\u01c3\7\3\2\2\u01c3\u01c7")
        buf.write("\5&\24\2\u01c4\u01c6\5D#\2\u01c5\u01c4\3\2\2\2\u01c6\u01c9")
        buf.write("\3\2\2\2\u01c7\u01c5\3\2\2\2\u01c7\u01c8\3\2\2\2\u01c8")
        buf.write("\u01ca\3\2\2\2\u01c9\u01c7\3\2\2\2\u01ca\u01cb\7\5\2\2")
        buf.write("\u01cbC\3\2\2\2\u01cc\u01cd\t\2\2\2\u01cdE\3\2\2\2\u01ce")
        buf.write("\u01cf\7\3\2\2\u01cf\u01d0\7\36\2\2\u01d0\u01d1\5\64\33")
        buf.write("\2\u01d1\u01d2\7\22\2\2\u01d2\u01d3\7\3\2\2\u01d3\u01d4")
        buf.write("\5(\25\2\u01d4\u01d5\7\5\2\2\u01d5\u01d6\5H%\2\u01d6\u01d7")
        buf.write("\7\5\2\2\u01d7\u01e2\3\2\2\2\u01d8\u01d9\7\3\2\2\u01d9")
        buf.write("\u01da\7\36\2\2\u01da\u01db\5\64\33\2\u01db\u01dc\7\22")
        buf.write("\2\2\u01dc\u01dd\7\3\2\2\u01dd\u01de\7\5\2\2\u01de\u01df")
        buf.write("\5H%\2\u01df\u01e0\7\5\2\2\u01e0\u01e2\3\2\2\2\u01e1\u01ce")
        buf.write("\3\2\2\2\u01e1\u01d8\3\2\2\2\u01e2G\3\2\2\2\u01e3\u01e4")
        buf.write("\7\37\2\2\u01e4\u01f2\5j\66\2\u01e5\u01e9\7\26\2\2\u01e6")
        buf.write("\u01e7\7\3\2\2\u01e7\u01ea\7\5\2\2\u01e8\u01ea\5J&\2\u01e9")
        buf.write("\u01e6\3\2\2\2\u01e9\u01e8\3\2\2\2\u01ea\u01f2\3\2\2\2")
        buf.write("\u01eb\u01ef\7\25\2\2\u01ec\u01ed\7\3\2\2\u01ed\u01f0")
        buf.write("\7\5\2\2\u01ee\u01f0\5r:\2\u01ef\u01ec\3\2\2\2\u01ef\u01ee")
        buf.write("\3\2\2\2\u01f0\u01f2\3\2\2\2\u01f1\u01e3\3\2\2\2\u01f1")
        buf.write("\u01e5\3\2\2\2\u01f1\u01eb\3\2\2\2\u01f2I\3\2\2\2\u01f3")
        buf.write("\u0206\5L\'\2\u01f4\u01f5\7\3\2\2\u01f5\u01f9\7\27\2\2")
        buf.write("\u01f6\u01f8\5J&\2\u01f7\u01f6\3\2\2\2\u01f8\u01fb\3\2")
        buf.write("\2\2\u01f9\u01f7\3\2\2\2\u01f9\u01fa\3\2\2\2\u01fa\u01fc")
        buf.write("\3\2\2\2\u01fb\u01f9\3\2\2\2\u01fc\u0206\7\5\2\2\u01fd")
        buf.write("\u01fe\7\3\2\2\u01fe\u01ff\7\34\2\2\u01ff\u0200\7\3\2")
        buf.write("\2\u0200\u0201\5(\25\2\u0201\u0202\7\5\2\2\u0202\u0203")
        buf.write("\5J&\2\u0203\u0204\7\5\2\2\u0204\u0206\3\2\2\2\u0205\u01f3")
        buf.write("\3\2\2\2\u0205\u01f4\3\2\2\2\u0205\u01fd\3\2\2\2\u0206")
        buf.write("K\3\2\2\2\u0207\u0211\5N(\2\u0208\u0209\7\3\2\2\u0209")
        buf.write("\u020b\7 \2\2\u020a\u020c\7H\2\2\u020b\u020a\3\2\2\2\u020b")
        buf.write("\u020c\3\2\2\2\u020c\u020d\3\2\2\2\u020d\u020e\5N(\2\u020e")
        buf.write("\u020f\7\5\2\2\u020f\u0211\3\2\2\2\u0210\u0207\3\2\2\2")
        buf.write("\u0210\u0208\3\2\2\2\u0211M\3\2\2\2\u0212\u0213\7\3\2")
        buf.write("\2\u0213\u0214\7!\2\2\u0214\u0215\5P)\2\u0215\u0216\5")
        buf.write("<\37\2\u0216\u0217\7\5\2\2\u0217\u021f\3\2\2\2\u0218\u0219")
        buf.write("\7\3\2\2\u0219\u021a\7\"\2\2\u021a\u021b\5R*\2\u021b\u021c")
        buf.write("\5<\37\2\u021c\u021d\7\5\2\2\u021d\u021f\3\2\2\2\u021e")
        buf.write("\u0212\3\2\2\2\u021e\u0218\3\2\2\2\u021fO\3\2\2\2\u0220")
        buf.write("\u0221\t\3\2\2\u0221Q\3\2\2\2\u0222\u0223\7%\2\2\u0223")
        buf.write("S\3\2\2\2\u0224\u0225\7\3\2\2\u0225\u0226\7&\2\2\u0226")
        buf.write("\u0227\5(\25\2\u0227\u0228\5<\37\2\u0228\u0229\7\5\2\2")
        buf.write("\u0229U\3\2\2\2\u022a\u0238\7J\2\2\u022b\u022c\7\3\2\2")
        buf.write("\u022c\u022d\5b\62\2\u022d\u022e\5V,\2\u022e\u022f\5V")
        buf.write(",\2\u022f\u0230\7\5\2\2\u0230\u0238\3\2\2\2\u0231\u0232")
        buf.write("\7\3\2\2\u0232\u0233\7\n\2\2\u0233\u0234\5V,\2\u0234\u0235")
        buf.write("\7\5\2\2\u0235\u0238\3\2\2\2\u0236\u0238\5X-\2\u0237\u022a")
        buf.write("\3\2\2\2\u0237\u022b\3\2\2\2\u0237\u0231\3\2\2\2\u0237")
        buf.write("\u0236\3\2\2\2\u0238W\3\2\2\2\u0239\u023a\7\3\2\2\u023a")
        buf.write("\u023e\5\34\17\2\u023b\u023d\5D#\2\u023c\u023b\3\2\2\2")
        buf.write("\u023d\u0240\3\2\2\2\u023e\u023c\3\2\2\2\u023e\u023f\3")
        buf.write("\2\2\2\u023f\u0241\3\2\2\2\u0240\u023e\3\2\2\2\u0241\u0242")
        buf.write("\7\5\2\2\u0242\u0245\3\2\2\2\u0243\u0245\5\34\17\2\u0244")
        buf.write("\u0239\3\2\2\2\u0244\u0243\3\2\2\2\u0245Y\3\2\2\2\u0246")
        buf.write("\u0247\7\3\2\2\u0247\u024b\7\27\2\2\u0248\u024a\5\\/\2")
        buf.write("\u0249\u0248\3\2\2\2\u024a\u024d\3\2\2\2\u024b\u0249\3")
        buf.write("\2\2\2\u024b\u024c\3\2\2\2\u024c\u024e\3\2\2\2\u024d\u024b")
        buf.write("\3\2\2\2\u024e\u0251\7\5\2\2\u024f\u0251\5\\/\2\u0250")
        buf.write("\u0246\3\2\2\2\u0250\u024f\3\2\2\2\u0251[\3\2\2\2\u0252")
        buf.write("\u0253\7\3\2\2\u0253\u0254\7\34\2\2\u0254\u0255\7\3\2")
        buf.write("\2\u0255\u0256\5(\25\2\u0256\u0257\7\5\2\2\u0257\u0258")
        buf.write("\5Z.\2\u0258\u0259\7\5\2\2\u0259\u0262\3\2\2\2\u025a\u025b")
        buf.write("\7\3\2\2\u025b\u025c\7\'\2\2\u025c\u025d\5<\37\2\u025d")
        buf.write("\u025e\5`\61\2\u025e\u025f\7\5\2\2\u025f\u0262\3\2\2\2")
        buf.write("\u0260\u0262\5^\60\2\u0261\u0252\3\2\2\2\u0261\u025a\3")
        buf.write("\2\2\2\u0261\u0260\3\2\2\2\u0262]\3\2\2\2\u0263\u0264")
        buf.write("\7\3\2\2\u0264\u0265\5h\65\2\u0265\u0266\5X-\2\u0266\u0267")
        buf.write("\5V,\2\u0267\u0268\7\5\2\2\u0268\u0270\3\2\2\2\u0269\u026a")
        buf.write("\7\3\2\2\u026a\u026b\7\31\2\2\u026b\u026c\5B\"\2\u026c")
        buf.write("\u026d\7\5\2\2\u026d\u0270\3\2\2\2\u026e\u0270\5B\"\2")
        buf.write("\u026f\u0263\3\2\2\2\u026f\u0269\3\2\2\2\u026f\u026e\3")
        buf.write("\2\2\2\u0270_\3\2\2\2\u0271\u0272\7\3\2\2\u0272\u0276")
        buf.write("\7\27\2\2\u0273\u0275\5^\60\2\u0274\u0273\3\2\2\2\u0275")
        buf.write("\u0278\3\2\2\2\u0276\u0274\3\2\2\2\u0276\u0277\3\2\2\2")
        buf.write("\u0277\u0279\3\2\2\2\u0278\u0276\3\2\2\2\u0279\u027c\7")
        buf.write("\5\2\2\u027a\u027c\5^\60\2\u027b\u0271\3\2\2\2\u027b\u027a")
        buf.write("\3\2\2\2\u027ca\3\2\2\2\u027d\u027e\t\4\2\2\u027ec\3\2")
        buf.write("\2\2\u027f\u0280\t\5\2\2\u0280e\3\2\2\2\u0281\u0282\t")
        buf.write("\6\2\2\u0282g\3\2\2\2\u0283\u0284\t\7\2\2\u0284i\3\2\2")
        buf.write("\2\u0285\u0286\7\3\2\2\u0286\u0288\7\27\2\2\u0287\u0289")
        buf.write("\5l\67\2\u0288\u0287\3\2\2\2\u0289\u028a\3\2\2\2\u028a")
        buf.write("\u0288\3\2\2\2\u028a\u028b\3\2\2\2\u028b\u028c\3\2\2\2")
        buf.write("\u028c\u028d\7\5\2\2\u028d\u0292\3\2\2\2\u028e\u028f\7")
        buf.write("\3\2\2\u028f\u0292\7\5\2\2\u0290\u0292\5l\67\2\u0291\u0285")
        buf.write("\3\2\2\2\u0291\u028e\3\2\2\2\u0291\u0290\3\2\2\2\u0292")
        buf.write("k\3\2\2\2\u0293\u0294\7\3\2\2\u0294\u0295\5n8\2\u0295")
        buf.write("\u0296\7\64\2\2\u0296\u0297\5p9\2\u0297\u0298\7\5\2\2")
        buf.write("\u0298\u02a0\3\2\2\2\u0299\u029a\7\3\2\2\u029a\u029b\7")
        buf.write("!\2\2\u029b\u029c\5P)\2\u029c\u029d\5l\67\2\u029d\u029e")
        buf.write("\7\5\2\2\u029e\u02a0\3\2\2\2\u029f\u0293\3\2\2\2\u029f")
        buf.write("\u0299\3\2\2\2\u02a0m\3\2\2\2\u02a1\u02a2\t\b\2\2\u02a2")
        buf.write("o\3\2\2\2\u02a3\u02a6\7J\2\2\u02a4\u02a6\5V,\2\u02a5\u02a3")
        buf.write("\3\2\2\2\u02a5\u02a4\3\2\2\2\u02a6q\3\2\2\2\u02a7\u02a8")
        buf.write("\7\3\2\2\u02a8\u02ac\7\27\2\2\u02a9\u02ab\5r:\2\u02aa")
        buf.write("\u02a9\3\2\2\2\u02ab\u02ae\3\2\2\2\u02ac\u02aa\3\2\2\2")
        buf.write("\u02ac\u02ad\3\2\2\2\u02ad\u02af\3\2\2\2\u02ae\u02ac\3")
        buf.write("\2\2\2\u02af\u02c6\7\5\2\2\u02b0\u02c6\5t;\2\u02b1\u02b2")
        buf.write("\7\3\2\2\u02b2\u02b3\7\34\2\2\u02b3\u02b4\7\3\2\2\u02b4")
        buf.write("\u02b5\5(\25\2\u02b5\u02b6\7\5\2\2\u02b6\u02b7\5r:\2\u02b7")
        buf.write("\u02b8\7\5\2\2\u02b8\u02c6\3\2\2\2\u02b9\u02ba\7\3\2\2")
        buf.write("\u02ba\u02bb\7\'\2\2\u02bb\u02bc\5J&\2\u02bc\u02bd\5t")
        buf.write(";\2\u02bd\u02be\7\5\2\2\u02be\u02c6\3\2\2\2\u02bf\u02c0")
        buf.write("\7\3\2\2\u02c0\u02c1\5h\65\2\u02c1\u02c2\5X-\2\u02c2\u02c3")
        buf.write("\5x=\2\u02c3\u02c4\7\5\2\2\u02c4\u02c6\3\2\2\2\u02c5\u02a7")
        buf.write("\3\2\2\2\u02c5\u02b0\3\2\2\2\u02c5\u02b1\3\2\2\2\u02c5")
        buf.write("\u02b9\3\2\2\2\u02c5\u02bf\3\2\2\2\u02c6s\3\2\2\2\u02c7")
        buf.write("\u02c8\7\3\2\2\u02c8\u02c9\7!\2\2\u02c9\u02ca\5P)\2\u02ca")
        buf.write("\u02cb\5r:\2\u02cb\u02cc\7\5\2\2\u02cc\u02da\3\2\2\2\u02cd")
        buf.write("\u02ce\7\3\2\2\u02ce\u02cf\7!\2\2\u02cf\u02d0\5P)\2\u02d0")
        buf.write("\u02d1\5v<\2\u02d1\u02d2\7\5\2\2\u02d2\u02da\3\2\2\2\u02d3")
        buf.write("\u02d4\7\3\2\2\u02d4\u02d5\5h\65\2\u02d5\u02d6\5X-\2\u02d6")
        buf.write("\u02d7\5V,\2\u02d7\u02d8\7\5\2\2\u02d8\u02da\3\2\2\2\u02d9")
        buf.write("\u02c7\3\2\2\2\u02d9\u02cd\3\2\2\2\u02d9\u02d3\3\2\2\2")
        buf.write("\u02dau\3\2\2\2\u02db\u02dc\7\3\2\2\u02dc\u02dd\5h\65")
        buf.write("\2\u02dd\u02de\5X-\2\u02de\u02df\5x=\2\u02df\u02e0\7\5")
        buf.write("\2\2\u02e0w\3\2\2\2\u02e1\u02e8\7\3\2\2\u02e2\u02e3\5")
        buf.write("b\62\2\u02e3\u02e4\5x=\2\u02e4\u02e5\5x=\2\u02e5\u02e9")
        buf.write("\3\2\2\2\u02e6\u02e7\7\n\2\2\u02e7\u02e9\5x=\2\u02e8\u02e2")
        buf.write("\3\2\2\2\u02e8\u02e6\3\2\2\2\u02e9\u02ea\3\2\2\2\u02ea")
        buf.write("\u02eb\7\5\2\2\u02eb\u02ef\3\2\2\2\u02ec\u02ef\7\64\2")
        buf.write("\2\u02ed\u02ef\5V,\2\u02ee\u02e1\3\2\2\2\u02ee\u02ec\3")
        buf.write("\2\2\2\u02ee\u02ed\3\2\2\2\u02efy\3\2\2\2\u02f0\u02f1")
        buf.write("\7\3\2\2\u02f1\u02f2\7\4\2\2\u02f2\u02f3\5|?\2\u02f3\u02f5")
        buf.write("\5~@\2\u02f4\u02f6\5\n\6\2\u02f5\u02f4\3\2\2\2\u02f5\u02f6")
        buf.write("\3\2\2\2\u02f6\u02f8\3\2\2\2\u02f7\u02f9\5\u0080A\2\u02f8")
        buf.write("\u02f7\3\2\2\2\u02f8\u02f9\3\2\2\2\u02f9\u02fa\3\2\2\2")
        buf.write("\u02fa\u02fb\5\u0082B\2\u02fb\u02fd\5\u008aF\2\u02fc\u02fe")
        buf.write("\5\u008cG\2\u02fd\u02fc\3\2\2\2\u02fd\u02fe\3\2\2\2\u02fe")
        buf.write("\u0300\3\2\2\2\u02ff\u0301\5\u0090I\2\u0300\u02ff\3\2")
        buf.write("\2\2\u0300\u0301\3\2\2\2\u0301\u0302\3\2\2\2\u0302\u0303")
        buf.write("\7\5\2\2\u0303{\3\2\2\2\u0304\u0305\7\3\2\2\u0305\u0306")
        buf.write("\7\65\2\2\u0306\u0307\7H\2\2\u0307\u0308\7\5\2\2\u0308")
        buf.write("}\3\2\2\2\u0309\u030a\7\3\2\2\u030a\u030b\7\66\2\2\u030b")
        buf.write("\u030c\7H\2\2\u030c\u030d\7\5\2\2\u030d\177\3\2\2\2\u030e")
        buf.write("\u030f\7\3\2\2\u030f\u0310\7\67\2\2\u0310\u0311\5\16\b")
        buf.write("\2\u0311\u0312\7\5\2\2\u0312\u0081\3\2\2\2\u0313\u0314")
        buf.write("\7\3\2\2\u0314\u0318\78\2\2\u0315\u0317\5\u0084C\2\u0316")
        buf.write("\u0315\3\2\2\2\u0317\u031a\3\2\2\2\u0318\u0316\3\2\2\2")
        buf.write("\u0318\u0319\3\2\2\2\u0319\u031b\3\2\2\2\u031a\u0318\3")
        buf.write("\2\2\2\u031b\u031c\7\5\2\2\u031c\u0083\3\2\2\2\u031d\u032b")
        buf.write("\5\u0086D\2\u031e\u031f\7\3\2\2\u031f\u0320\7\35\2\2\u0320")
        buf.write("\u0321\5X-\2\u0321\u0322\7J\2\2\u0322\u0323\7\5\2\2\u0323")
        buf.write("\u032b\3\2\2\2\u0324\u0325\7\3\2\2\u0325\u0326\7!\2\2")
        buf.write("\u0326\u0327\7J\2\2\u0327\u0328\5\u0086D\2\u0328\u0329")
        buf.write("\7\5\2\2\u0329\u032b\3\2\2\2\u032a\u031d\3\2\2\2\u032a")
        buf.write("\u031e\3\2\2\2\u032a\u0324\3\2\2\2\u032b\u0085\3\2\2\2")
        buf.write("\u032c\u0333\5\u0088E\2\u032d\u032e\7\3\2\2\u032e\u032f")
        buf.write("\7\31\2\2\u032f\u0330\5\u0088E\2\u0330\u0331\7\5\2\2\u0331")
        buf.write("\u0333\3\2\2\2\u0332\u032c\3\2\2\2\u0332\u032d\3\2\2\2")
        buf.write("\u0333\u0087\3\2\2\2\u0334\u0335\7\3\2\2\u0335\u0339\5")
        buf.write("&\24\2\u0336\u0338\7H\2\2\u0337\u0336\3\2\2\2\u0338\u033b")
        buf.write("\3\2\2\2\u0339\u0337\3\2\2\2\u0339\u033a\3\2\2\2\u033a")
        buf.write("\u033c\3\2\2\2\u033b\u0339\3\2\2\2\u033c\u033d\7\5\2\2")
        buf.write("\u033d\u0089\3\2\2\2\u033e\u033f\7\3\2\2\u033f\u0340\7")
        buf.write("9\2\2\u0340\u0341\5<\37\2\u0341\u0342\7\5\2\2\u0342\u008b")
        buf.write("\3\2\2\2\u0343\u0344\7\3\2\2\u0344\u0345\7\20\2\2\u0345")
        buf.write("\u0346\5\u008eH\2\u0346\u0347\7\5\2\2\u0347\u008d\3\2")
        buf.write("\2\2\u0348\u0349\7\3\2\2\u0349\u034d\7\27\2\2\u034a\u034c")
        buf.write("\5\u008eH\2\u034b\u034a\3\2\2\2\u034c\u034f\3\2\2\2\u034d")
        buf.write("\u034b\3\2\2\2\u034d\u034e\3\2\2\2\u034e\u0350\3\2\2\2")
        buf.write("\u034f\u034d\3\2\2\2\u0350\u0363\7\5\2\2\u0351\u0352\7")
        buf.write("\3\2\2\u0352\u0353\7\34\2\2\u0353\u0354\7\3\2\2\u0354")
        buf.write("\u0355\5(\25\2\u0355\u0356\7\5\2\2\u0356\u0357\5\u008e")
        buf.write("H\2\u0357\u0358\7\5\2\2\u0358\u0363\3\2\2\2\u0359\u035a")
        buf.write("\7\3\2\2\u035a\u035c\7 \2\2\u035b\u035d\7H\2\2\u035c\u035b")
        buf.write("\3\2\2\2\u035c\u035d\3\2\2\2\u035d\u035e\3\2\2\2\u035e")
        buf.write("\u035f\5\u0096L\2\u035f\u0360\7\5\2\2\u0360\u0363\3\2")
        buf.write("\2\2\u0361\u0363\5\u0096L\2\u0362\u0348\3\2\2\2\u0362")
        buf.write("\u0351\3\2\2\2\u0362\u0359\3\2\2\2\u0362\u0361\3\2\2\2")
        buf.write("\u0363\u008f\3\2\2\2\u0364\u0365\7\3\2\2\u0365\u0366\7")
        buf.write(":\2\2\u0366\u0367\5\u0092J\2\u0367\u0368\5\u0094K\2\u0368")
        buf.write("\u0369\7\5\2\2\u0369\u0091\3\2\2\2\u036a\u036b\t\t\2\2")
        buf.write("\u036b\u0093\3\2\2\2\u036c\u036d\7\3\2\2\u036d\u036e\5")
        buf.write("b\62\2\u036e\u036f\5\u0094K\2\u036f\u0370\5\u0094K\2\u0370")
        buf.write("\u0371\7\5\2\2\u0371\u0388\3\2\2\2\u0372\u0373\7\3\2\2")
        buf.write("\u0373\u0374\5d\63\2\u0374\u0376\5\u0094K\2\u0375\u0377")
        buf.write("\5\u0094K\2\u0376\u0375\3\2\2\2\u0377\u0378\3\2\2\2\u0378")
        buf.write("\u0376\3\2\2\2\u0378\u0379\3\2\2\2\u0379\u037a\3\2\2\2")
        buf.write("\u037a\u037b\7\5\2\2\u037b\u0388\3\2\2\2\u037c\u037d\7")
        buf.write("\3\2\2\u037d\u037e\7\n\2\2\u037e\u037f\5\u0094K\2\u037f")
        buf.write("\u0380\7\5\2\2\u0380\u0388\3\2\2\2\u0381\u0388\7J\2\2")
        buf.write("\u0382\u0388\5X-\2\u0383\u0384\7\3\2\2\u0384\u0385\7=")
        buf.write("\2\2\u0385\u0386\7H\2\2\u0386\u0388\7\5\2\2\u0387\u036c")
        buf.write("\3\2\2\2\u0387\u0372\3\2\2\2\u0387\u037c\3\2\2\2\u0387")
        buf.write("\u0381\3\2\2\2\u0387\u0382\3\2\2\2\u0387\u0383\3\2\2\2")
        buf.write("\u0388\u0095\3\2\2\2\u0389\u038a\7\3\2\2\u038a\u038e\7")
        buf.write("\27\2\2\u038b\u038d\5\u0096L\2\u038c\u038b\3\2\2\2\u038d")
        buf.write("\u0390\3\2\2\2\u038e\u038c\3\2\2\2\u038e\u038f\3\2\2\2")
        buf.write("\u038f\u0391\3\2\2\2\u0390\u038e\3\2\2\2\u0391\u03d6\7")
        buf.write("\5\2\2\u0392\u0393\7\3\2\2\u0393\u0394\7\34\2\2\u0394")
        buf.write("\u0395\7\3\2\2\u0395\u0396\5(\25\2\u0396\u0397\7\5\2\2")
        buf.write("\u0397\u0398\5\u0096L\2\u0398\u0399\7\5\2\2\u0399\u03d6")
        buf.write("\3\2\2\2\u039a\u039b\7\3\2\2\u039b\u039c\7!\2\2\u039c")
        buf.write("\u039d\7$\2\2\u039d\u039e\5<\37\2\u039e\u039f\7\5\2\2")
        buf.write("\u039f\u03d6\3\2\2\2\u03a0\u03a1\7\3\2\2\u03a1\u03a2\7")
        buf.write(">\2\2\u03a2\u03a3\5<\37\2\u03a3\u03a4\7\5\2\2\u03a4\u03d6")
        buf.write("\3\2\2\2\u03a5\u03a6\7\3\2\2\u03a6\u03a7\7?\2\2\u03a7")
        buf.write("\u03a8\5<\37\2\u03a8\u03a9\7\5\2\2\u03a9\u03d6\3\2\2\2")
        buf.write("\u03aa\u03ab\7\3\2\2\u03ab\u03ac\7@\2\2\u03ac\u03ad\7")
        buf.write("J\2\2\u03ad\u03ae\5<\37\2\u03ae\u03af\7\5\2\2\u03af\u03d6")
        buf.write("\3\2\2\2\u03b0\u03b1\7\3\2\2\u03b1\u03b2\7A\2\2\u03b2")
        buf.write("\u03b3\5<\37\2\u03b3\u03b4\7\5\2\2\u03b4\u03d6\3\2\2\2")
        buf.write("\u03b5\u03b6\7\3\2\2\u03b6\u03b7\7B\2\2\u03b7\u03b8\5")
        buf.write("<\37\2\u03b8\u03b9\5<\37\2\u03b9\u03ba\7\5\2\2\u03ba\u03d6")
        buf.write("\3\2\2\2\u03bb\u03bc\7\3\2\2\u03bc\u03bd\7C\2\2\u03bd")
        buf.write("\u03be\5<\37\2\u03be\u03bf\5<\37\2\u03bf\u03c0\7\5\2\2")
        buf.write("\u03c0\u03d6\3\2\2\2\u03c1\u03c2\7\3\2\2\u03c2\u03c3\7")
        buf.write("D\2\2\u03c3\u03c4\7J\2\2\u03c4\u03c5\5<\37\2\u03c5\u03c6")
        buf.write("\5<\37\2\u03c6\u03c7\7\5\2\2\u03c7\u03d6\3\2\2\2\u03c8")
        buf.write("\u03c9\7\3\2\2\u03c9\u03ca\7E\2\2\u03ca\u03cb\7J\2\2\u03cb")
        buf.write("\u03cc\7J\2\2\u03cc\u03cd\5<\37\2\u03cd\u03ce\7\5\2\2")
        buf.write("\u03ce\u03d6\3\2\2\2\u03cf\u03d0\7\3\2\2\u03d0\u03d1\7")
        buf.write("F\2\2\u03d1\u03d2\7J\2\2\u03d2\u03d3\5<\37\2\u03d3\u03d4")
        buf.write("\7\5\2\2\u03d4\u03d6\3\2\2\2\u03d5\u0389\3\2\2\2\u03d5")
        buf.write("\u0392\3\2\2\2\u03d5\u039a\3\2\2\2\u03d5\u03a0\3\2\2\2")
        buf.write("\u03d5\u03a5\3\2\2\2\u03d5\u03aa\3\2\2\2\u03d5\u03b0\3")
        buf.write("\2\2\2\u03d5\u03b5\3\2\2\2\u03d5\u03bb\3\2\2\2\u03d5\u03c1")
        buf.write("\3\2\2\2\u03d5\u03c8\3\2\2\2\u03d5\u03cf\3\2\2\2\u03d6")
        buf.write("\u0097\3\2\2\2Q\u009a\u00a0\u00a3\u00a6\u00a9\u00ac\u00af")
        buf.write("\u00b2\u00b7\u00cb\u00d7\u00dd\u00e2\u00e5\u00ea\u00f4")
        buf.write("\u00f9\u0105\u0109\u010d\u0123\u0131\u0137\u013c\u013f")
        buf.write("\u0144\u0152\u0170\u0172\u0178\u017a\u0180\u0182\u018a")
        buf.write("\u0193\u01b4\u01c7\u01e1\u01e9\u01ef\u01f1\u01f9\u0205")
        buf.write("\u020b\u0210\u021e\u0237\u023e\u0244\u024b\u0250\u0261")
        buf.write("\u026f\u0276\u027b\u028a\u0291\u029f\u02a5\u02ac\u02c5")
        buf.write("\u02d9\u02e8\u02ee\u02f5\u02f8\u02fd\u0300\u0318\u032a")
        buf.write("\u0332\u0339\u034d\u035c\u0362\u0378\u0387\u038e\u03d5")
        return buf.getvalue()


class Pddl3Parser ( Parser ):

    grammarFileName = "java-escape"

    atn = ATNDeserializer().deserialize(serializedATN())

    decisionsToDFA = [ DFA(ds, i) for i, ds in enumerate(atn.decisionToState) ]

    sharedContextCache = PredictionContextCache()

    literalNames = [ u"<INVALID>", u"'('", u"'define'", u"')'", u"':free_functions'", 
                     u"'domain'", u"':requirements'", u"':types'", u"'-'", 
                     u"'either'", u"':functions'", u"'number'", u"':constants'", 
                     u"':predicates'", u"':constraints'", u"':action'", 
                     u"':parameters'", u"':constraint'", u"':precondition'", 
                     u"':effect'", u"':condition'", u"'and'", u"'or'", u"'not'", 
                     u"'imply'", u"'exists'", u"'forall'", u"'='", u"':durative-action'", 
                     u"':duration'", u"'preference'", u"'at'", u"'over'", 
                     u"'start'", u"'end'", u"'all'", u"':derived'", u"'when'", 
                     u"'*'", u"'+'", u"'/'", u"'>'", u"'<'", u"'>='", u"'<='", 
                     u"'assign'", u"'scale-up'", u"'scale-down'", u"'increase'", 
                     u"'decrease'", u"'?duration'", u"'problem'", u"':domain'", 
                     u"':objects'", u"':init'", u"':goal'", u"':metric'", 
                     u"'minimize'", u"'maximize'", u"'is-violated'", u"'always'", 
                     u"'sometime'", u"'within'", u"'at-most-once'", u"'sometime-after'", 
                     u"'sometime-before'", u"'always-within'", u"'hold-during'", 
                     u"'hold-after'" ]

    symbolicNames = [ u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"REQUIRE_KEY", u"NAME", u"VARIABLE", 
                      u"NUMBER", u"LINE_COMMENT", u"WHITESPACE", u"DOMAIN", 
                      u"DOMAIN_NAME", u"REQUIREMENTS", u"TYPES", u"EITHER_TYPE", 
                      u"CONSTANTS", u"FUNCTIONS", u"FREE_FUNCTIONS", u"PREDICATES", 
                      u"ACTION", u"CONSTRAINT", u"GLOBAL_CONSTRAINT", u"DURATIVE_ACTION", 
                      u"PROBLEM", u"PROBLEM_NAME", u"PROBLEM_DOMAIN", u"OBJECTS", 
                      u"INIT", u"FUNC_HEAD", u"PRECONDITION", u"EFFECT", 
                      u"AND_GD", u"OR_GD", u"NOT_GD", u"IMPLY_GD", u"EXISTS_GD", 
                      u"FORALL_GD", u"COMPARISON_GD", u"AND_EFFECT", u"FORALL_EFFECT", 
                      u"WHEN_EFFECT", u"ASSIGN_EFFECT", u"NOT_EFFECT", u"PRED_HEAD", 
                      u"GOAL", u"BINARY_OP", u"EQUALITY_CON", u"MULTI_OP", 
                      u"MINUS_OP", u"UNARY_MINUS", u"INIT_EQ", u"INIT_AT", 
                      u"NOT_PRED_INIT", u"PRED_INST", u"PROBLEM_CONSTRAINT", 
                      u"PROBLEM_METRIC" ]

    RULE_pddlDoc = 0
    RULE_domain = 1
    RULE_free_functionsDef = 2
    RULE_domainName = 3
    RULE_requireDef = 4
    RULE_typesDef = 5
    RULE_typedNameList = 6
    RULE_singleTypeNameList = 7
    RULE_typename = 8
    RULE_primType = 9
    RULE_functionsDef = 10
    RULE_functionList = 11
    RULE_atomicFunctionSkeleton = 12
    RULE_functionSymbol = 13
    RULE_functionType = 14
    RULE_constantsDef = 15
    RULE_predicatesDef = 16
    RULE_atomicFormulaSkeleton = 17
    RULE_predicate = 18
    RULE_typedVariableList = 19
    RULE_singleTypeVarList = 20
    RULE_constraints = 21
    RULE_structureDef = 22
    RULE_actionDef = 23
    RULE_constraintDef = 24
    RULE_actionSymbol = 25
    RULE_constraintSymbol = 26
    RULE_actionDefBody = 27
    RULE_constraintDefBody = 28
    RULE_goalDesc = 29
    RULE_equality = 30
    RULE_fComp = 31
    RULE_atomicTermFormula = 32
    RULE_term = 33
    RULE_durativeActionDef = 34
    RULE_daDefBody = 35
    RULE_daGD = 36
    RULE_prefTimedGD = 37
    RULE_timedGD = 38
    RULE_timeSpecifier = 39
    RULE_interval = 40
    RULE_derivedDef = 41
    RULE_fExp = 42
    RULE_fHead = 43
    RULE_effect = 44
    RULE_cEffect = 45
    RULE_pEffect = 46
    RULE_condEffect = 47
    RULE_binaryOp = 48
    RULE_multiOp = 49
    RULE_binaryComp = 50
    RULE_assignOp = 51
    RULE_durationConstraint = 52
    RULE_simpleDurationConstraint = 53
    RULE_durOp = 54
    RULE_durValue = 55
    RULE_daEffect = 56
    RULE_timedEffect = 57
    RULE_fAssignDA = 58
    RULE_fExpDA = 59
    RULE_problem = 60
    RULE_problemDecl = 61
    RULE_problemDomain = 62
    RULE_objectDecl = 63
    RULE_init = 64
    RULE_initEl = 65
    RULE_nameLiteral = 66
    RULE_atomicNameFormula = 67
    RULE_goal = 68
    RULE_probConstraints = 69
    RULE_prefConGD = 70
    RULE_metricSpec = 71
    RULE_optimization = 72
    RULE_metricFExp = 73
    RULE_conGD = 74

    ruleNames =  [ "pddlDoc", "domain", "free_functionsDef", "domainName", 
                   "requireDef", "typesDef", "typedNameList", "singleTypeNameList", 
                   "typename", "primType", "functionsDef", "functionList", 
                   "atomicFunctionSkeleton", "functionSymbol", "functionType", 
                   "constantsDef", "predicatesDef", "atomicFormulaSkeleton", 
                   "predicate", "typedVariableList", "singleTypeVarList", 
                   "constraints", "structureDef", "actionDef", "constraintDef", 
                   "actionSymbol", "constraintSymbol", "actionDefBody", 
                   "constraintDefBody", "goalDesc", "equality", "fComp", 
                   "atomicTermFormula", "term", "durativeActionDef", "daDefBody", 
                   "daGD", "prefTimedGD", "timedGD", "timeSpecifier", "interval", 
                   "derivedDef", "fExp", "fHead", "effect", "cEffect", "pEffect", 
                   "condEffect", "binaryOp", "multiOp", "binaryComp", "assignOp", 
                   "durationConstraint", "simpleDurationConstraint", "durOp", 
                   "durValue", "daEffect", "timedEffect", "fAssignDA", "fExpDA", 
                   "problem", "problemDecl", "problemDomain", "objectDecl", 
                   "init", "initEl", "nameLiteral", "atomicNameFormula", 
                   "goal", "probConstraints", "prefConGD", "metricSpec", 
                   "optimization", "metricFExp", "conGD" ]

    EOF = Token.EOF
    T__0=1
    T__1=2
    T__2=3
    T__3=4
    T__4=5
    T__5=6
    T__6=7
    T__7=8
    T__8=9
    T__9=10
    T__10=11
    T__11=12
    T__12=13
    T__13=14
    T__14=15
    T__15=16
    T__16=17
    T__17=18
    T__18=19
    T__19=20
    T__20=21
    T__21=22
    T__22=23
    T__23=24
    T__24=25
    T__25=26
    T__26=27
    T__27=28
    T__28=29
    T__29=30
    T__30=31
    T__31=32
    T__32=33
    T__33=34
    T__34=35
    T__35=36
    T__36=37
    T__37=38
    T__38=39
    T__39=40
    T__40=41
    T__41=42
    T__42=43
    T__43=44
    T__44=45
    T__45=46
    T__46=47
    T__47=48
    T__48=49
    T__49=50
    T__50=51
    T__51=52
    T__52=53
    T__53=54
    T__54=55
    T__55=56
    T__56=57
    T__57=58
    T__58=59
    T__59=60
    T__60=61
    T__61=62
    T__62=63
    T__63=64
    T__64=65
    T__65=66
    T__66=67
    T__67=68
    REQUIRE_KEY=69
    NAME=70
    VARIABLE=71
    NUMBER=72
    LINE_COMMENT=73
    WHITESPACE=74
    DOMAIN=75
    DOMAIN_NAME=76
    REQUIREMENTS=77
    TYPES=78
    EITHER_TYPE=79
    CONSTANTS=80
    FUNCTIONS=81
    FREE_FUNCTIONS=82
    PREDICATES=83
    ACTION=84
    CONSTRAINT=85
    GLOBAL_CONSTRAINT=86
    DURATIVE_ACTION=87
    PROBLEM=88
    PROBLEM_NAME=89
    PROBLEM_DOMAIN=90
    OBJECTS=91
    INIT=92
    FUNC_HEAD=93
    PRECONDITION=94
    EFFECT=95
    AND_GD=96
    OR_GD=97
    NOT_GD=98
    IMPLY_GD=99
    EXISTS_GD=100
    FORALL_GD=101
    COMPARISON_GD=102
    AND_EFFECT=103
    FORALL_EFFECT=104
    WHEN_EFFECT=105
    ASSIGN_EFFECT=106
    NOT_EFFECT=107
    PRED_HEAD=108
    GOAL=109
    BINARY_OP=110
    EQUALITY_CON=111
    MULTI_OP=112
    MINUS_OP=113
    UNARY_MINUS=114
    INIT_EQ=115
    INIT_AT=116
    NOT_PRED_INIT=117
    PRED_INST=118
    PROBLEM_CONSTRAINT=119
    PROBLEM_METRIC=120

    def __init__(self, input:TokenStream):
        super().__init__(input)
        self.checkVersion("4.5")
        self._interp = ParserATNSimulator(self, self.atn, self.decisionsToDFA, self.sharedContextCache)
        self._predicates = None



    class PddlDocContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def domain(self):
            return self.getTypedRuleContext(Pddl3Parser.DomainContext,0)


        def problem(self):
            return self.getTypedRuleContext(Pddl3Parser.ProblemContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_pddlDoc

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterPddlDoc(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitPddlDoc(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitPddlDoc(self)
            else:
                return visitor.visitChildren(self)




    def pddlDoc(self):

        localctx = Pddl3Parser.PddlDocContext(self, self._ctx, self.state)
        self.enterRule(localctx, 0, self.RULE_pddlDoc)
        try:
            self.state = 152
            la_ = self._interp.adaptivePredict(self._input,0,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 150
                self.domain()
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 151
                self.problem()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DomainContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def domainName(self):
            return self.getTypedRuleContext(Pddl3Parser.DomainNameContext,0)


        def requireDef(self):
            return self.getTypedRuleContext(Pddl3Parser.RequireDefContext,0)


        def typesDef(self):
            return self.getTypedRuleContext(Pddl3Parser.TypesDefContext,0)


        def constantsDef(self):
            return self.getTypedRuleContext(Pddl3Parser.ConstantsDefContext,0)


        def predicatesDef(self):
            return self.getTypedRuleContext(Pddl3Parser.PredicatesDefContext,0)


        def functionsDef(self):
            return self.getTypedRuleContext(Pddl3Parser.FunctionsDefContext,0)


        def free_functionsDef(self):
            return self.getTypedRuleContext(Pddl3Parser.Free_functionsDefContext,0)


        def constraints(self):
            return self.getTypedRuleContext(Pddl3Parser.ConstraintsContext,0)


        def structureDef(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.StructureDefContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.StructureDefContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_domain

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDomain(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDomain(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDomain(self)
            else:
                return visitor.visitChildren(self)




    def domain(self):

        localctx = Pddl3Parser.DomainContext(self, self._ctx, self.state)
        self.enterRule(localctx, 2, self.RULE_domain)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 154
            self.match(Pddl3Parser.T__0)
            self.state = 155
            self.match(Pddl3Parser.T__1)
            self.state = 156
            self.domainName()
            self.state = 158
            la_ = self._interp.adaptivePredict(self._input,1,self._ctx)
            if la_ == 1:
                self.state = 157
                self.requireDef()


            self.state = 161
            la_ = self._interp.adaptivePredict(self._input,2,self._ctx)
            if la_ == 1:
                self.state = 160
                self.typesDef()


            self.state = 164
            la_ = self._interp.adaptivePredict(self._input,3,self._ctx)
            if la_ == 1:
                self.state = 163
                self.constantsDef()


            self.state = 167
            la_ = self._interp.adaptivePredict(self._input,4,self._ctx)
            if la_ == 1:
                self.state = 166
                self.predicatesDef()


            self.state = 170
            la_ = self._interp.adaptivePredict(self._input,5,self._ctx)
            if la_ == 1:
                self.state = 169
                self.functionsDef()


            self.state = 173
            la_ = self._interp.adaptivePredict(self._input,6,self._ctx)
            if la_ == 1:
                self.state = 172
                self.free_functionsDef()


            self.state = 176
            la_ = self._interp.adaptivePredict(self._input,7,self._ctx)
            if la_ == 1:
                self.state = 175
                self.constraints()


            self.state = 181
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==Pddl3Parser.T__0:
                self.state = 178
                self.structureDef()
                self.state = 183
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 184
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class Free_functionsDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def functionList(self):
            return self.getTypedRuleContext(Pddl3Parser.FunctionListContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_free_functionsDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFree_functionsDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFree_functionsDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFree_functionsDef(self)
            else:
                return visitor.visitChildren(self)




    def free_functionsDef(self):

        localctx = Pddl3Parser.Free_functionsDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 4, self.RULE_free_functionsDef)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 186
            self.match(Pddl3Parser.T__0)
            self.state = 187
            self.match(Pddl3Parser.T__3)
            self.state = 188
            self.functionList()
            self.state = 189
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DomainNameContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_domainName

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDomainName(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDomainName(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDomainName(self)
            else:
                return visitor.visitChildren(self)




    def domainName(self):

        localctx = Pddl3Parser.DomainNameContext(self, self._ctx, self.state)
        self.enterRule(localctx, 6, self.RULE_domainName)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 191
            self.match(Pddl3Parser.T__0)
            self.state = 192
            self.match(Pddl3Parser.T__4)
            self.state = 193
            self.match(Pddl3Parser.NAME)
            self.state = 194
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class RequireDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def REQUIRE_KEY(self, i:int=None):
            if i is None:
                return self.getTokens(Pddl3Parser.REQUIRE_KEY)
            else:
                return self.getToken(Pddl3Parser.REQUIRE_KEY, i)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_requireDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterRequireDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitRequireDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitRequireDef(self)
            else:
                return visitor.visitChildren(self)




    def requireDef(self):

        localctx = Pddl3Parser.RequireDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 8, self.RULE_requireDef)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 196
            self.match(Pddl3Parser.T__0)
            self.state = 197
            self.match(Pddl3Parser.T__5)
            self.state = 199 
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while True:
                self.state = 198
                self.match(Pddl3Parser.REQUIRE_KEY)
                self.state = 201 
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                if not (_la==Pddl3Parser.REQUIRE_KEY):
                    break

            self.state = 203
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class TypesDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def typedNameList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedNameListContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_typesDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterTypesDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitTypesDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitTypesDef(self)
            else:
                return visitor.visitChildren(self)




    def typesDef(self):

        localctx = Pddl3Parser.TypesDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 10, self.RULE_typesDef)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 205
            self.match(Pddl3Parser.T__0)
            self.state = 206
            self.match(Pddl3Parser.T__6)
            self.state = 207
            self.typedNameList()
            self.state = 208
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class TypedNameListContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self, i:int=None):
            if i is None:
                return self.getTokens(Pddl3Parser.NAME)
            else:
                return self.getToken(Pddl3Parser.NAME, i)

        def singleTypeNameList(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.SingleTypeNameListContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.SingleTypeNameListContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_typedNameList

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterTypedNameList(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitTypedNameList(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitTypedNameList(self)
            else:
                return visitor.visitChildren(self)




    def typedNameList(self):

        localctx = Pddl3Parser.TypedNameListContext(self, self._ctx, self.state)
        self.enterRule(localctx, 12, self.RULE_typedNameList)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 227
            la_ = self._interp.adaptivePredict(self._input,13,self._ctx)
            if la_ == 1:
                self.state = 213
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.NAME:
                    self.state = 210
                    self.match(Pddl3Parser.NAME)
                    self.state = 215
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                pass

            elif la_ == 2:
                self.state = 217 
                self._errHandler.sync(self)
                _alt = 1
                while _alt!=2 and _alt!=ATN.INVALID_ALT_NUMBER:
                    if _alt == 1:
                        self.state = 216
                        self.singleTypeNameList()

                    else:
                        raise NoViableAltException(self)
                    self.state = 219 
                    self._errHandler.sync(self)
                    _alt = self._interp.adaptivePredict(self._input,11,self._ctx)

                self.state = 224
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.NAME:
                    self.state = 221
                    self.match(Pddl3Parser.NAME)
                    self.state = 226
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class SingleTypeNameListContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser
            self.t = None # TypenameContext

        def typename(self):
            return self.getTypedRuleContext(Pddl3Parser.TypenameContext,0)


        def NAME(self, i:int=None):
            if i is None:
                return self.getTokens(Pddl3Parser.NAME)
            else:
                return self.getToken(Pddl3Parser.NAME, i)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_singleTypeNameList

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterSingleTypeNameList(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitSingleTypeNameList(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitSingleTypeNameList(self)
            else:
                return visitor.visitChildren(self)




    def singleTypeNameList(self):

        localctx = Pddl3Parser.SingleTypeNameListContext(self, self._ctx, self.state)
        self.enterRule(localctx, 14, self.RULE_singleTypeNameList)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 230 
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while True:
                self.state = 229
                self.match(Pddl3Parser.NAME)
                self.state = 232 
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                if not (_la==Pddl3Parser.NAME):
                    break

            self.state = 234
            self.match(Pddl3Parser.T__7)
            self.state = 235
            localctx.t = self.typename()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class TypenameContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def primType(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.PrimTypeContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.PrimTypeContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_typename

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterTypename(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitTypename(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitTypename(self)
            else:
                return visitor.visitChildren(self)




    def typename(self):

        localctx = Pddl3Parser.TypenameContext(self, self._ctx, self.state)
        self.enterRule(localctx, 16, self.RULE_typename)
        self._la = 0 # Token type
        try:
            self.state = 247
            token = self._input.LA(1)
            if token in [Pddl3Parser.T__0]:
                self.enterOuterAlt(localctx, 1)
                self.state = 237
                self.match(Pddl3Parser.T__0)
                self.state = 238
                self.match(Pddl3Parser.T__8)
                self.state = 240 
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while True:
                    self.state = 239
                    self.primType()
                    self.state = 242 
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)
                    if not (_la==Pddl3Parser.NAME):
                        break

                self.state = 244
                self.match(Pddl3Parser.T__2)

            elif token in [Pddl3Parser.NAME]:
                self.enterOuterAlt(localctx, 2)
                self.state = 246
                self.primType()

            else:
                raise NoViableAltException(self)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class PrimTypeContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_primType

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterPrimType(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitPrimType(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitPrimType(self)
            else:
                return visitor.visitChildren(self)




    def primType(self):

        localctx = Pddl3Parser.PrimTypeContext(self, self._ctx, self.state)
        self.enterRule(localctx, 18, self.RULE_primType)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 249
            self.match(Pddl3Parser.NAME)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FunctionsDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def functionList(self):
            return self.getTypedRuleContext(Pddl3Parser.FunctionListContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_functionsDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFunctionsDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFunctionsDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFunctionsDef(self)
            else:
                return visitor.visitChildren(self)




    def functionsDef(self):

        localctx = Pddl3Parser.FunctionsDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 20, self.RULE_functionsDef)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 251
            self.match(Pddl3Parser.T__0)
            self.state = 252
            self.match(Pddl3Parser.T__9)
            self.state = 253
            self.functionList()
            self.state = 254
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FunctionListContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def atomicFunctionSkeleton(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.AtomicFunctionSkeletonContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.AtomicFunctionSkeletonContext,i)


        def functionType(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.FunctionTypeContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.FunctionTypeContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_functionList

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFunctionList(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFunctionList(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFunctionList(self)
            else:
                return visitor.visitChildren(self)




    def functionList(self):

        localctx = Pddl3Parser.FunctionListContext(self, self._ctx, self.state)
        self.enterRule(localctx, 22, self.RULE_functionList)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 267
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==Pddl3Parser.T__0:
                self.state = 257 
                self._errHandler.sync(self)
                _alt = 1
                while _alt!=2 and _alt!=ATN.INVALID_ALT_NUMBER:
                    if _alt == 1:
                        self.state = 256
                        self.atomicFunctionSkeleton()

                    else:
                        raise NoViableAltException(self)
                    self.state = 259 
                    self._errHandler.sync(self)
                    _alt = self._interp.adaptivePredict(self._input,17,self._ctx)

                self.state = 263
                _la = self._input.LA(1)
                if _la==Pddl3Parser.T__7:
                    self.state = 261
                    self.match(Pddl3Parser.T__7)
                    self.state = 262
                    self.functionType()


                self.state = 269
                self._errHandler.sync(self)
                _la = self._input.LA(1)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class AtomicFunctionSkeletonContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def functionSymbol(self):
            return self.getTypedRuleContext(Pddl3Parser.FunctionSymbolContext,0)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_atomicFunctionSkeleton

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterAtomicFunctionSkeleton(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitAtomicFunctionSkeleton(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitAtomicFunctionSkeleton(self)
            else:
                return visitor.visitChildren(self)




    def atomicFunctionSkeleton(self):

        localctx = Pddl3Parser.AtomicFunctionSkeletonContext(self, self._ctx, self.state)
        self.enterRule(localctx, 24, self.RULE_atomicFunctionSkeleton)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 270
            self.match(Pddl3Parser.T__0)
            self.state = 271
            self.functionSymbol()
            self.state = 272
            self.typedVariableList()
            self.state = 273
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FunctionSymbolContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_functionSymbol

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFunctionSymbol(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFunctionSymbol(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFunctionSymbol(self)
            else:
                return visitor.visitChildren(self)




    def functionSymbol(self):

        localctx = Pddl3Parser.FunctionSymbolContext(self, self._ctx, self.state)
        self.enterRule(localctx, 26, self.RULE_functionSymbol)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 275
            self.match(Pddl3Parser.NAME)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FunctionTypeContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return Pddl3Parser.RULE_functionType

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFunctionType(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFunctionType(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFunctionType(self)
            else:
                return visitor.visitChildren(self)




    def functionType(self):

        localctx = Pddl3Parser.FunctionTypeContext(self, self._ctx, self.state)
        self.enterRule(localctx, 28, self.RULE_functionType)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 277
            self.match(Pddl3Parser.T__10)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ConstantsDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def typedNameList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedNameListContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_constantsDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterConstantsDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitConstantsDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitConstantsDef(self)
            else:
                return visitor.visitChildren(self)




    def constantsDef(self):

        localctx = Pddl3Parser.ConstantsDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 30, self.RULE_constantsDef)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 279
            self.match(Pddl3Parser.T__0)
            self.state = 280
            self.match(Pddl3Parser.T__11)
            self.state = 281
            self.typedNameList()
            self.state = 282
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class PredicatesDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def atomicFormulaSkeleton(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.AtomicFormulaSkeletonContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.AtomicFormulaSkeletonContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_predicatesDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterPredicatesDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitPredicatesDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitPredicatesDef(self)
            else:
                return visitor.visitChildren(self)




    def predicatesDef(self):

        localctx = Pddl3Parser.PredicatesDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 32, self.RULE_predicatesDef)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 284
            self.match(Pddl3Parser.T__0)
            self.state = 285
            self.match(Pddl3Parser.T__12)
            self.state = 287 
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while True:
                self.state = 286
                self.atomicFormulaSkeleton()
                self.state = 289 
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                if not (_la==Pddl3Parser.T__0):
                    break

            self.state = 291
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class AtomicFormulaSkeletonContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def predicate(self):
            return self.getTypedRuleContext(Pddl3Parser.PredicateContext,0)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_atomicFormulaSkeleton

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterAtomicFormulaSkeleton(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitAtomicFormulaSkeleton(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitAtomicFormulaSkeleton(self)
            else:
                return visitor.visitChildren(self)




    def atomicFormulaSkeleton(self):

        localctx = Pddl3Parser.AtomicFormulaSkeletonContext(self, self._ctx, self.state)
        self.enterRule(localctx, 34, self.RULE_atomicFormulaSkeleton)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 293
            self.match(Pddl3Parser.T__0)
            self.state = 294
            self.predicate()
            self.state = 295
            self.typedVariableList()
            self.state = 296
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class PredicateContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_predicate

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterPredicate(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitPredicate(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitPredicate(self)
            else:
                return visitor.visitChildren(self)




    def predicate(self):

        localctx = Pddl3Parser.PredicateContext(self, self._ctx, self.state)
        self.enterRule(localctx, 36, self.RULE_predicate)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 298
            self.match(Pddl3Parser.NAME)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class TypedVariableListContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def VARIABLE(self, i:int=None):
            if i is None:
                return self.getTokens(Pddl3Parser.VARIABLE)
            else:
                return self.getToken(Pddl3Parser.VARIABLE, i)

        def singleTypeVarList(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.SingleTypeVarListContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.SingleTypeVarListContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_typedVariableList

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterTypedVariableList(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitTypedVariableList(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitTypedVariableList(self)
            else:
                return visitor.visitChildren(self)




    def typedVariableList(self):

        localctx = Pddl3Parser.TypedVariableListContext(self, self._ctx, self.state)
        self.enterRule(localctx, 38, self.RULE_typedVariableList)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 317
            la_ = self._interp.adaptivePredict(self._input,24,self._ctx)
            if la_ == 1:
                self.state = 303
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.VARIABLE:
                    self.state = 300
                    self.match(Pddl3Parser.VARIABLE)
                    self.state = 305
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                pass

            elif la_ == 2:
                self.state = 307 
                self._errHandler.sync(self)
                _alt = 1
                while _alt!=2 and _alt!=ATN.INVALID_ALT_NUMBER:
                    if _alt == 1:
                        self.state = 306
                        self.singleTypeVarList()

                    else:
                        raise NoViableAltException(self)
                    self.state = 309 
                    self._errHandler.sync(self)
                    _alt = self._interp.adaptivePredict(self._input,22,self._ctx)

                self.state = 314
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.VARIABLE:
                    self.state = 311
                    self.match(Pddl3Parser.VARIABLE)
                    self.state = 316
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class SingleTypeVarListContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser
            self.t = None # TypenameContext

        def typename(self):
            return self.getTypedRuleContext(Pddl3Parser.TypenameContext,0)


        def VARIABLE(self, i:int=None):
            if i is None:
                return self.getTokens(Pddl3Parser.VARIABLE)
            else:
                return self.getToken(Pddl3Parser.VARIABLE, i)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_singleTypeVarList

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterSingleTypeVarList(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitSingleTypeVarList(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitSingleTypeVarList(self)
            else:
                return visitor.visitChildren(self)




    def singleTypeVarList(self):

        localctx = Pddl3Parser.SingleTypeVarListContext(self, self._ctx, self.state)
        self.enterRule(localctx, 40, self.RULE_singleTypeVarList)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 320 
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while True:
                self.state = 319
                self.match(Pddl3Parser.VARIABLE)
                self.state = 322 
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                if not (_la==Pddl3Parser.VARIABLE):
                    break

            self.state = 324
            self.match(Pddl3Parser.T__7)
            self.state = 325
            localctx.t = self.typename()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ConstraintsContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def conGD(self):
            return self.getTypedRuleContext(Pddl3Parser.ConGDContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_constraints

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterConstraints(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitConstraints(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitConstraints(self)
            else:
                return visitor.visitChildren(self)




    def constraints(self):

        localctx = Pddl3Parser.ConstraintsContext(self, self._ctx, self.state)
        self.enterRule(localctx, 42, self.RULE_constraints)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 327
            self.match(Pddl3Parser.T__0)
            self.state = 328
            self.match(Pddl3Parser.T__13)
            self.state = 329
            self.conGD()
            self.state = 330
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class StructureDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def actionDef(self):
            return self.getTypedRuleContext(Pddl3Parser.ActionDefContext,0)


        def durativeActionDef(self):
            return self.getTypedRuleContext(Pddl3Parser.DurativeActionDefContext,0)


        def derivedDef(self):
            return self.getTypedRuleContext(Pddl3Parser.DerivedDefContext,0)


        def constraintDef(self):
            return self.getTypedRuleContext(Pddl3Parser.ConstraintDefContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_structureDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterStructureDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitStructureDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitStructureDef(self)
            else:
                return visitor.visitChildren(self)




    def structureDef(self):

        localctx = Pddl3Parser.StructureDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 44, self.RULE_structureDef)
        try:
            self.state = 336
            la_ = self._interp.adaptivePredict(self._input,26,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 332
                self.actionDef()
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 333
                self.durativeActionDef()
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 334
                self.derivedDef()
                pass

            elif la_ == 4:
                self.enterOuterAlt(localctx, 4)
                self.state = 335
                self.constraintDef()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ActionDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def actionSymbol(self):
            return self.getTypedRuleContext(Pddl3Parser.ActionSymbolContext,0)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def actionDefBody(self):
            return self.getTypedRuleContext(Pddl3Parser.ActionDefBodyContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_actionDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterActionDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitActionDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitActionDef(self)
            else:
                return visitor.visitChildren(self)




    def actionDef(self):

        localctx = Pddl3Parser.ActionDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 46, self.RULE_actionDef)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 338
            self.match(Pddl3Parser.T__0)
            self.state = 339
            self.match(Pddl3Parser.T__14)
            self.state = 340
            self.actionSymbol()
            self.state = 341
            self.match(Pddl3Parser.T__15)
            self.state = 342
            self.match(Pddl3Parser.T__0)
            self.state = 343
            self.typedVariableList()
            self.state = 344
            self.match(Pddl3Parser.T__2)
            self.state = 345
            self.actionDefBody()
            self.state = 346
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ConstraintDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def constraintSymbol(self):
            return self.getTypedRuleContext(Pddl3Parser.ConstraintSymbolContext,0)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def constraintDefBody(self):
            return self.getTypedRuleContext(Pddl3Parser.ConstraintDefBodyContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_constraintDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterConstraintDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitConstraintDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitConstraintDef(self)
            else:
                return visitor.visitChildren(self)




    def constraintDef(self):

        localctx = Pddl3Parser.ConstraintDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 48, self.RULE_constraintDef)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 348
            self.match(Pddl3Parser.T__0)
            self.state = 349
            self.match(Pddl3Parser.T__16)
            self.state = 350
            self.constraintSymbol()
            self.state = 351
            self.match(Pddl3Parser.T__15)
            self.state = 352
            self.match(Pddl3Parser.T__0)
            self.state = 353
            self.typedVariableList()
            self.state = 354
            self.match(Pddl3Parser.T__2)
            self.state = 355
            self.constraintDefBody()
            self.state = 356
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ActionSymbolContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_actionSymbol

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterActionSymbol(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitActionSymbol(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitActionSymbol(self)
            else:
                return visitor.visitChildren(self)




    def actionSymbol(self):

        localctx = Pddl3Parser.ActionSymbolContext(self, self._ctx, self.state)
        self.enterRule(localctx, 50, self.RULE_actionSymbol)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 358
            self.match(Pddl3Parser.NAME)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ConstraintSymbolContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_constraintSymbol

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterConstraintSymbol(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitConstraintSymbol(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitConstraintSymbol(self)
            else:
                return visitor.visitChildren(self)




    def constraintSymbol(self):

        localctx = Pddl3Parser.ConstraintSymbolContext(self, self._ctx, self.state)
        self.enterRule(localctx, 52, self.RULE_constraintSymbol)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 360
            self.match(Pddl3Parser.NAME)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ActionDefBodyContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def goalDesc(self):
            return self.getTypedRuleContext(Pddl3Parser.GoalDescContext,0)


        def effect(self):
            return self.getTypedRuleContext(Pddl3Parser.EffectContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_actionDefBody

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterActionDefBody(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitActionDefBody(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitActionDefBody(self)
            else:
                return visitor.visitChildren(self)




    def actionDefBody(self):

        localctx = Pddl3Parser.ActionDefBodyContext(self, self._ctx, self.state)
        self.enterRule(localctx, 54, self.RULE_actionDefBody)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 368
            _la = self._input.LA(1)
            if _la==Pddl3Parser.T__17:
                self.state = 362
                self.match(Pddl3Parser.T__17)
                self.state = 366
                la_ = self._interp.adaptivePredict(self._input,27,self._ctx)
                if la_ == 1:
                    self.state = 363
                    self.match(Pddl3Parser.T__0)
                    self.state = 364
                    self.match(Pddl3Parser.T__2)
                    pass

                elif la_ == 2:
                    self.state = 365
                    self.goalDesc()
                    pass




            self.state = 376
            _la = self._input.LA(1)
            if _la==Pddl3Parser.T__18:
                self.state = 370
                self.match(Pddl3Parser.T__18)
                self.state = 374
                la_ = self._interp.adaptivePredict(self._input,29,self._ctx)
                if la_ == 1:
                    self.state = 371
                    self.match(Pddl3Parser.T__0)
                    self.state = 372
                    self.match(Pddl3Parser.T__2)
                    pass

                elif la_ == 2:
                    self.state = 373
                    self.effect()
                    pass




        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ConstraintDefBodyContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def goalDesc(self):
            return self.getTypedRuleContext(Pddl3Parser.GoalDescContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_constraintDefBody

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterConstraintDefBody(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitConstraintDefBody(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitConstraintDefBody(self)
            else:
                return visitor.visitChildren(self)




    def constraintDefBody(self):

        localctx = Pddl3Parser.ConstraintDefBodyContext(self, self._ctx, self.state)
        self.enterRule(localctx, 56, self.RULE_constraintDefBody)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 384
            _la = self._input.LA(1)
            if _la==Pddl3Parser.T__19:
                self.state = 378
                self.match(Pddl3Parser.T__19)
                self.state = 382
                la_ = self._interp.adaptivePredict(self._input,31,self._ctx)
                if la_ == 1:
                    self.state = 379
                    self.match(Pddl3Parser.T__0)
                    self.state = 380
                    self.match(Pddl3Parser.T__2)
                    pass

                elif la_ == 2:
                    self.state = 381
                    self.goalDesc()
                    pass




        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class GoalDescContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def atomicTermFormula(self):
            return self.getTypedRuleContext(Pddl3Parser.AtomicTermFormulaContext,0)


        def goalDesc(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.GoalDescContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.GoalDescContext,i)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def fComp(self):
            return self.getTypedRuleContext(Pddl3Parser.FCompContext,0)


        def equality(self):
            return self.getTypedRuleContext(Pddl3Parser.EqualityContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_goalDesc

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterGoalDesc(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitGoalDesc(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitGoalDesc(self)
            else:
                return visitor.visitChildren(self)




    def goalDesc(self):

        localctx = Pddl3Parser.GoalDescContext(self, self._ctx, self.state)
        self.enterRule(localctx, 58, self.RULE_goalDesc)
        self._la = 0 # Token type
        try:
            self.state = 434
            la_ = self._interp.adaptivePredict(self._input,35,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 386
                self.atomicTermFormula()
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 387
                self.match(Pddl3Parser.T__0)
                self.state = 388
                self.match(Pddl3Parser.T__20)
                self.state = 392
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.T__0:
                    self.state = 389
                    self.goalDesc()
                    self.state = 394
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 395
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 396
                self.match(Pddl3Parser.T__0)
                self.state = 397
                self.match(Pddl3Parser.T__21)
                self.state = 401
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.T__0:
                    self.state = 398
                    self.goalDesc()
                    self.state = 403
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 404
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 4:
                self.enterOuterAlt(localctx, 4)
                self.state = 405
                self.match(Pddl3Parser.T__0)
                self.state = 406
                self.match(Pddl3Parser.T__22)
                self.state = 407
                self.goalDesc()
                self.state = 408
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 5:
                self.enterOuterAlt(localctx, 5)
                self.state = 410
                self.match(Pddl3Parser.T__0)
                self.state = 411
                self.match(Pddl3Parser.T__23)
                self.state = 412
                self.goalDesc()
                self.state = 413
                self.goalDesc()
                self.state = 414
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 6:
                self.enterOuterAlt(localctx, 6)
                self.state = 416
                self.match(Pddl3Parser.T__0)
                self.state = 417
                self.match(Pddl3Parser.T__24)
                self.state = 418
                self.match(Pddl3Parser.T__0)
                self.state = 419
                self.typedVariableList()
                self.state = 420
                self.match(Pddl3Parser.T__2)
                self.state = 421
                self.goalDesc()
                self.state = 422
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 7:
                self.enterOuterAlt(localctx, 7)
                self.state = 424
                self.match(Pddl3Parser.T__0)
                self.state = 425
                self.match(Pddl3Parser.T__25)
                self.state = 426
                self.match(Pddl3Parser.T__0)
                self.state = 427
                self.typedVariableList()
                self.state = 428
                self.match(Pddl3Parser.T__2)
                self.state = 429
                self.goalDesc()
                self.state = 430
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 8:
                self.enterOuterAlt(localctx, 8)
                self.state = 432
                self.fComp()
                pass

            elif la_ == 9:
                self.enterOuterAlt(localctx, 9)
                self.state = 433
                self.equality()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class EqualityContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def term(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.TermContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.TermContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_equality

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterEquality(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitEquality(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitEquality(self)
            else:
                return visitor.visitChildren(self)




    def equality(self):

        localctx = Pddl3Parser.EqualityContext(self, self._ctx, self.state)
        self.enterRule(localctx, 60, self.RULE_equality)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 436
            self.match(Pddl3Parser.T__0)
            self.state = 437
            self.match(Pddl3Parser.T__26)
            self.state = 438
            self.term()
            self.state = 439
            self.term()
            self.state = 440
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FCompContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def binaryComp(self):
            return self.getTypedRuleContext(Pddl3Parser.BinaryCompContext,0)


        def fExp(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.FExpContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.FExpContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_fComp

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFComp(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFComp(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFComp(self)
            else:
                return visitor.visitChildren(self)




    def fComp(self):

        localctx = Pddl3Parser.FCompContext(self, self._ctx, self.state)
        self.enterRule(localctx, 62, self.RULE_fComp)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 442
            self.match(Pddl3Parser.T__0)
            self.state = 443
            self.binaryComp()
            self.state = 444
            self.fExp()
            self.state = 445
            self.fExp()
            self.state = 446
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class AtomicTermFormulaContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def predicate(self):
            return self.getTypedRuleContext(Pddl3Parser.PredicateContext,0)


        def term(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.TermContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.TermContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_atomicTermFormula

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterAtomicTermFormula(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitAtomicTermFormula(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitAtomicTermFormula(self)
            else:
                return visitor.visitChildren(self)




    def atomicTermFormula(self):

        localctx = Pddl3Parser.AtomicTermFormulaContext(self, self._ctx, self.state)
        self.enterRule(localctx, 64, self.RULE_atomicTermFormula)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 448
            self.match(Pddl3Parser.T__0)
            self.state = 449
            self.predicate()
            self.state = 453
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==Pddl3Parser.NAME or _la==Pddl3Parser.VARIABLE:
                self.state = 450
                self.term()
                self.state = 455
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 456
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class TermContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def VARIABLE(self):
            return self.getToken(Pddl3Parser.VARIABLE, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_term

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterTerm(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitTerm(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitTerm(self)
            else:
                return visitor.visitChildren(self)




    def term(self):

        localctx = Pddl3Parser.TermContext(self, self._ctx, self.state)
        self.enterRule(localctx, 66, self.RULE_term)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 458
            _la = self._input.LA(1)
            if not(_la==Pddl3Parser.NAME or _la==Pddl3Parser.VARIABLE):
                self._errHandler.recoverInline(self)
            else:
                self.consume()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DurativeActionDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def actionSymbol(self):
            return self.getTypedRuleContext(Pddl3Parser.ActionSymbolContext,0)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def daDefBody(self):
            return self.getTypedRuleContext(Pddl3Parser.DaDefBodyContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_durativeActionDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDurativeActionDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDurativeActionDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDurativeActionDef(self)
            else:
                return visitor.visitChildren(self)




    def durativeActionDef(self):

        localctx = Pddl3Parser.DurativeActionDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 68, self.RULE_durativeActionDef)
        try:
            self.state = 479
            la_ = self._interp.adaptivePredict(self._input,37,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 460
                self.match(Pddl3Parser.T__0)
                self.state = 461
                self.match(Pddl3Parser.T__27)
                self.state = 462
                self.actionSymbol()
                self.state = 463
                self.match(Pddl3Parser.T__15)
                self.state = 464
                self.match(Pddl3Parser.T__0)
                self.state = 465
                self.typedVariableList()
                self.state = 466
                self.match(Pddl3Parser.T__2)
                self.state = 467
                self.daDefBody()
                self.state = 468
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 470
                self.match(Pddl3Parser.T__0)
                self.state = 471
                self.match(Pddl3Parser.T__27)
                self.state = 472
                self.actionSymbol()
                self.state = 473
                self.match(Pddl3Parser.T__15)
                self.state = 474
                self.match(Pddl3Parser.T__0)
                self.state = 475
                self.match(Pddl3Parser.T__2)
                self.state = 476
                self.daDefBody()
                self.state = 477
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DaDefBodyContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def durationConstraint(self):
            return self.getTypedRuleContext(Pddl3Parser.DurationConstraintContext,0)


        def daGD(self):
            return self.getTypedRuleContext(Pddl3Parser.DaGDContext,0)


        def daEffect(self):
            return self.getTypedRuleContext(Pddl3Parser.DaEffectContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_daDefBody

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDaDefBody(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDaDefBody(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDaDefBody(self)
            else:
                return visitor.visitChildren(self)




    def daDefBody(self):

        localctx = Pddl3Parser.DaDefBodyContext(self, self._ctx, self.state)
        self.enterRule(localctx, 70, self.RULE_daDefBody)
        try:
            self.state = 495
            token = self._input.LA(1)
            if token in [Pddl3Parser.T__28]:
                self.enterOuterAlt(localctx, 1)
                self.state = 481
                self.match(Pddl3Parser.T__28)
                self.state = 482
                self.durationConstraint()

            elif token in [Pddl3Parser.T__19]:
                self.enterOuterAlt(localctx, 2)
                self.state = 483
                self.match(Pddl3Parser.T__19)
                self.state = 487
                la_ = self._interp.adaptivePredict(self._input,38,self._ctx)
                if la_ == 1:
                    self.state = 484
                    self.match(Pddl3Parser.T__0)
                    self.state = 485
                    self.match(Pddl3Parser.T__2)
                    pass

                elif la_ == 2:
                    self.state = 486
                    self.daGD()
                    pass



            elif token in [Pddl3Parser.T__18]:
                self.enterOuterAlt(localctx, 3)
                self.state = 489
                self.match(Pddl3Parser.T__18)
                self.state = 493
                la_ = self._interp.adaptivePredict(self._input,39,self._ctx)
                if la_ == 1:
                    self.state = 490
                    self.match(Pddl3Parser.T__0)
                    self.state = 491
                    self.match(Pddl3Parser.T__2)
                    pass

                elif la_ == 2:
                    self.state = 492
                    self.daEffect()
                    pass



            else:
                raise NoViableAltException(self)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DaGDContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def prefTimedGD(self):
            return self.getTypedRuleContext(Pddl3Parser.PrefTimedGDContext,0)


        def daGD(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.DaGDContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.DaGDContext,i)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_daGD

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDaGD(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDaGD(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDaGD(self)
            else:
                return visitor.visitChildren(self)




    def daGD(self):

        localctx = Pddl3Parser.DaGDContext(self, self._ctx, self.state)
        self.enterRule(localctx, 72, self.RULE_daGD)
        self._la = 0 # Token type
        try:
            self.state = 515
            la_ = self._interp.adaptivePredict(self._input,42,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 497
                self.prefTimedGD()
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 498
                self.match(Pddl3Parser.T__0)
                self.state = 499
                self.match(Pddl3Parser.T__20)
                self.state = 503
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.T__0:
                    self.state = 500
                    self.daGD()
                    self.state = 505
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 506
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 507
                self.match(Pddl3Parser.T__0)
                self.state = 508
                self.match(Pddl3Parser.T__25)
                self.state = 509
                self.match(Pddl3Parser.T__0)
                self.state = 510
                self.typedVariableList()
                self.state = 511
                self.match(Pddl3Parser.T__2)
                self.state = 512
                self.daGD()
                self.state = 513
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class PrefTimedGDContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def timedGD(self):
            return self.getTypedRuleContext(Pddl3Parser.TimedGDContext,0)


        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_prefTimedGD

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterPrefTimedGD(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitPrefTimedGD(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitPrefTimedGD(self)
            else:
                return visitor.visitChildren(self)




    def prefTimedGD(self):

        localctx = Pddl3Parser.PrefTimedGDContext(self, self._ctx, self.state)
        self.enterRule(localctx, 74, self.RULE_prefTimedGD)
        self._la = 0 # Token type
        try:
            self.state = 526
            la_ = self._interp.adaptivePredict(self._input,44,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 517
                self.timedGD()
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 518
                self.match(Pddl3Parser.T__0)
                self.state = 519
                self.match(Pddl3Parser.T__29)
                self.state = 521
                _la = self._input.LA(1)
                if _la==Pddl3Parser.NAME:
                    self.state = 520
                    self.match(Pddl3Parser.NAME)


                self.state = 523
                self.timedGD()
                self.state = 524
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class TimedGDContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def timeSpecifier(self):
            return self.getTypedRuleContext(Pddl3Parser.TimeSpecifierContext,0)


        def goalDesc(self):
            return self.getTypedRuleContext(Pddl3Parser.GoalDescContext,0)


        def interval(self):
            return self.getTypedRuleContext(Pddl3Parser.IntervalContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_timedGD

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterTimedGD(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitTimedGD(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitTimedGD(self)
            else:
                return visitor.visitChildren(self)




    def timedGD(self):

        localctx = Pddl3Parser.TimedGDContext(self, self._ctx, self.state)
        self.enterRule(localctx, 76, self.RULE_timedGD)
        try:
            self.state = 540
            la_ = self._interp.adaptivePredict(self._input,45,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 528
                self.match(Pddl3Parser.T__0)
                self.state = 529
                self.match(Pddl3Parser.T__30)
                self.state = 530
                self.timeSpecifier()
                self.state = 531
                self.goalDesc()
                self.state = 532
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 534
                self.match(Pddl3Parser.T__0)
                self.state = 535
                self.match(Pddl3Parser.T__31)
                self.state = 536
                self.interval()
                self.state = 537
                self.goalDesc()
                self.state = 538
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class TimeSpecifierContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return Pddl3Parser.RULE_timeSpecifier

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterTimeSpecifier(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitTimeSpecifier(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitTimeSpecifier(self)
            else:
                return visitor.visitChildren(self)




    def timeSpecifier(self):

        localctx = Pddl3Parser.TimeSpecifierContext(self, self._ctx, self.state)
        self.enterRule(localctx, 78, self.RULE_timeSpecifier)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 542
            _la = self._input.LA(1)
            if not(_la==Pddl3Parser.T__32 or _la==Pddl3Parser.T__33):
                self._errHandler.recoverInline(self)
            else:
                self.consume()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class IntervalContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return Pddl3Parser.RULE_interval

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterInterval(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitInterval(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitInterval(self)
            else:
                return visitor.visitChildren(self)




    def interval(self):

        localctx = Pddl3Parser.IntervalContext(self, self._ctx, self.state)
        self.enterRule(localctx, 80, self.RULE_interval)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 544
            self.match(Pddl3Parser.T__34)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DerivedDefContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def goalDesc(self):
            return self.getTypedRuleContext(Pddl3Parser.GoalDescContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_derivedDef

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDerivedDef(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDerivedDef(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDerivedDef(self)
            else:
                return visitor.visitChildren(self)




    def derivedDef(self):

        localctx = Pddl3Parser.DerivedDefContext(self, self._ctx, self.state)
        self.enterRule(localctx, 82, self.RULE_derivedDef)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 546
            self.match(Pddl3Parser.T__0)
            self.state = 547
            self.match(Pddl3Parser.T__35)
            self.state = 548
            self.typedVariableList()
            self.state = 549
            self.goalDesc()
            self.state = 550
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FExpContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NUMBER(self):
            return self.getToken(Pddl3Parser.NUMBER, 0)

        def binaryOp(self):
            return self.getTypedRuleContext(Pddl3Parser.BinaryOpContext,0)


        def fExp(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.FExpContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.FExpContext,i)


        def fHead(self):
            return self.getTypedRuleContext(Pddl3Parser.FHeadContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_fExp

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFExp(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFExp(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFExp(self)
            else:
                return visitor.visitChildren(self)




    def fExp(self):

        localctx = Pddl3Parser.FExpContext(self, self._ctx, self.state)
        self.enterRule(localctx, 84, self.RULE_fExp)
        try:
            self.state = 565
            la_ = self._interp.adaptivePredict(self._input,46,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 552
                self.match(Pddl3Parser.NUMBER)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 553
                self.match(Pddl3Parser.T__0)
                self.state = 554
                self.binaryOp()
                self.state = 555
                self.fExp()
                self.state = 556
                self.fExp()
                self.state = 557
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 559
                self.match(Pddl3Parser.T__0)
                self.state = 560
                self.match(Pddl3Parser.T__7)
                self.state = 561
                self.fExp()
                self.state = 562
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 4:
                self.enterOuterAlt(localctx, 4)
                self.state = 564
                self.fHead()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FHeadContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def functionSymbol(self):
            return self.getTypedRuleContext(Pddl3Parser.FunctionSymbolContext,0)


        def term(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.TermContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.TermContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_fHead

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFHead(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFHead(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFHead(self)
            else:
                return visitor.visitChildren(self)




    def fHead(self):

        localctx = Pddl3Parser.FHeadContext(self, self._ctx, self.state)
        self.enterRule(localctx, 86, self.RULE_fHead)
        self._la = 0 # Token type
        try:
            self.state = 578
            token = self._input.LA(1)
            if token in [Pddl3Parser.T__0]:
                self.enterOuterAlt(localctx, 1)
                self.state = 567
                self.match(Pddl3Parser.T__0)
                self.state = 568
                self.functionSymbol()
                self.state = 572
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.NAME or _la==Pddl3Parser.VARIABLE:
                    self.state = 569
                    self.term()
                    self.state = 574
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 575
                self.match(Pddl3Parser.T__2)

            elif token in [Pddl3Parser.NAME]:
                self.enterOuterAlt(localctx, 2)
                self.state = 577
                self.functionSymbol()

            else:
                raise NoViableAltException(self)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class EffectContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def cEffect(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.CEffectContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.CEffectContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_effect

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterEffect(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitEffect(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitEffect(self)
            else:
                return visitor.visitChildren(self)




    def effect(self):

        localctx = Pddl3Parser.EffectContext(self, self._ctx, self.state)
        self.enterRule(localctx, 88, self.RULE_effect)
        self._la = 0 # Token type
        try:
            self.state = 590
            la_ = self._interp.adaptivePredict(self._input,50,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 580
                self.match(Pddl3Parser.T__0)
                self.state = 581
                self.match(Pddl3Parser.T__20)
                self.state = 585
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.T__0:
                    self.state = 582
                    self.cEffect()
                    self.state = 587
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 588
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 589
                self.cEffect()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class CEffectContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def effect(self):
            return self.getTypedRuleContext(Pddl3Parser.EffectContext,0)


        def goalDesc(self):
            return self.getTypedRuleContext(Pddl3Parser.GoalDescContext,0)


        def condEffect(self):
            return self.getTypedRuleContext(Pddl3Parser.CondEffectContext,0)


        def pEffect(self):
            return self.getTypedRuleContext(Pddl3Parser.PEffectContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_cEffect

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterCEffect(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitCEffect(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitCEffect(self)
            else:
                return visitor.visitChildren(self)




    def cEffect(self):

        localctx = Pddl3Parser.CEffectContext(self, self._ctx, self.state)
        self.enterRule(localctx, 90, self.RULE_cEffect)
        try:
            self.state = 607
            la_ = self._interp.adaptivePredict(self._input,51,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 592
                self.match(Pddl3Parser.T__0)
                self.state = 593
                self.match(Pddl3Parser.T__25)
                self.state = 594
                self.match(Pddl3Parser.T__0)
                self.state = 595
                self.typedVariableList()
                self.state = 596
                self.match(Pddl3Parser.T__2)
                self.state = 597
                self.effect()
                self.state = 598
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 600
                self.match(Pddl3Parser.T__0)
                self.state = 601
                self.match(Pddl3Parser.T__36)
                self.state = 602
                self.goalDesc()
                self.state = 603
                self.condEffect()
                self.state = 604
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 606
                self.pEffect()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class PEffectContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def assignOp(self):
            return self.getTypedRuleContext(Pddl3Parser.AssignOpContext,0)


        def fHead(self):
            return self.getTypedRuleContext(Pddl3Parser.FHeadContext,0)


        def fExp(self):
            return self.getTypedRuleContext(Pddl3Parser.FExpContext,0)


        def atomicTermFormula(self):
            return self.getTypedRuleContext(Pddl3Parser.AtomicTermFormulaContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_pEffect

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterPEffect(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitPEffect(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitPEffect(self)
            else:
                return visitor.visitChildren(self)




    def pEffect(self):

        localctx = Pddl3Parser.PEffectContext(self, self._ctx, self.state)
        self.enterRule(localctx, 92, self.RULE_pEffect)
        try:
            self.state = 621
            la_ = self._interp.adaptivePredict(self._input,52,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 609
                self.match(Pddl3Parser.T__0)
                self.state = 610
                self.assignOp()
                self.state = 611
                self.fHead()
                self.state = 612
                self.fExp()
                self.state = 613
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 615
                self.match(Pddl3Parser.T__0)
                self.state = 616
                self.match(Pddl3Parser.T__22)
                self.state = 617
                self.atomicTermFormula()
                self.state = 618
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 620
                self.atomicTermFormula()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class CondEffectContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def pEffect(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.PEffectContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.PEffectContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_condEffect

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterCondEffect(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitCondEffect(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitCondEffect(self)
            else:
                return visitor.visitChildren(self)




    def condEffect(self):

        localctx = Pddl3Parser.CondEffectContext(self, self._ctx, self.state)
        self.enterRule(localctx, 94, self.RULE_condEffect)
        self._la = 0 # Token type
        try:
            self.state = 633
            la_ = self._interp.adaptivePredict(self._input,54,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 623
                self.match(Pddl3Parser.T__0)
                self.state = 624
                self.match(Pddl3Parser.T__20)
                self.state = 628
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.T__0:
                    self.state = 625
                    self.pEffect()
                    self.state = 630
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 631
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 632
                self.pEffect()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class BinaryOpContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return Pddl3Parser.RULE_binaryOp

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterBinaryOp(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitBinaryOp(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitBinaryOp(self)
            else:
                return visitor.visitChildren(self)




    def binaryOp(self):

        localctx = Pddl3Parser.BinaryOpContext(self, self._ctx, self.state)
        self.enterRule(localctx, 96, self.RULE_binaryOp)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 635
            _la = self._input.LA(1)
            if not((((_la) & ~0x3f) == 0 and ((1 << _la) & ((1 << Pddl3Parser.T__7) | (1 << Pddl3Parser.T__37) | (1 << Pddl3Parser.T__38) | (1 << Pddl3Parser.T__39))) != 0)):
                self._errHandler.recoverInline(self)
            else:
                self.consume()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class MultiOpContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return Pddl3Parser.RULE_multiOp

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterMultiOp(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitMultiOp(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitMultiOp(self)
            else:
                return visitor.visitChildren(self)




    def multiOp(self):

        localctx = Pddl3Parser.MultiOpContext(self, self._ctx, self.state)
        self.enterRule(localctx, 98, self.RULE_multiOp)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 637
            _la = self._input.LA(1)
            if not(_la==Pddl3Parser.T__37 or _la==Pddl3Parser.T__38):
                self._errHandler.recoverInline(self)
            else:
                self.consume()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class BinaryCompContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return Pddl3Parser.RULE_binaryComp

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterBinaryComp(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitBinaryComp(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitBinaryComp(self)
            else:
                return visitor.visitChildren(self)




    def binaryComp(self):

        localctx = Pddl3Parser.BinaryCompContext(self, self._ctx, self.state)
        self.enterRule(localctx, 100, self.RULE_binaryComp)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 639
            _la = self._input.LA(1)
            if not((((_la) & ~0x3f) == 0 and ((1 << _la) & ((1 << Pddl3Parser.T__26) | (1 << Pddl3Parser.T__40) | (1 << Pddl3Parser.T__41) | (1 << Pddl3Parser.T__42) | (1 << Pddl3Parser.T__43))) != 0)):
                self._errHandler.recoverInline(self)
            else:
                self.consume()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class AssignOpContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return Pddl3Parser.RULE_assignOp

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterAssignOp(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitAssignOp(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitAssignOp(self)
            else:
                return visitor.visitChildren(self)




    def assignOp(self):

        localctx = Pddl3Parser.AssignOpContext(self, self._ctx, self.state)
        self.enterRule(localctx, 102, self.RULE_assignOp)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 641
            _la = self._input.LA(1)
            if not((((_la) & ~0x3f) == 0 and ((1 << _la) & ((1 << Pddl3Parser.T__44) | (1 << Pddl3Parser.T__45) | (1 << Pddl3Parser.T__46) | (1 << Pddl3Parser.T__47) | (1 << Pddl3Parser.T__48))) != 0)):
                self._errHandler.recoverInline(self)
            else:
                self.consume()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DurationConstraintContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def simpleDurationConstraint(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.SimpleDurationConstraintContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.SimpleDurationConstraintContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_durationConstraint

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDurationConstraint(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDurationConstraint(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDurationConstraint(self)
            else:
                return visitor.visitChildren(self)




    def durationConstraint(self):

        localctx = Pddl3Parser.DurationConstraintContext(self, self._ctx, self.state)
        self.enterRule(localctx, 104, self.RULE_durationConstraint)
        self._la = 0 # Token type
        try:
            self.state = 655
            la_ = self._interp.adaptivePredict(self._input,56,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 643
                self.match(Pddl3Parser.T__0)
                self.state = 644
                self.match(Pddl3Parser.T__20)
                self.state = 646 
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while True:
                    self.state = 645
                    self.simpleDurationConstraint()
                    self.state = 648 
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)
                    if not (_la==Pddl3Parser.T__0):
                        break

                self.state = 650
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 652
                self.match(Pddl3Parser.T__0)
                self.state = 653
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 654
                self.simpleDurationConstraint()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class SimpleDurationConstraintContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def durOp(self):
            return self.getTypedRuleContext(Pddl3Parser.DurOpContext,0)


        def durValue(self):
            return self.getTypedRuleContext(Pddl3Parser.DurValueContext,0)


        def timeSpecifier(self):
            return self.getTypedRuleContext(Pddl3Parser.TimeSpecifierContext,0)


        def simpleDurationConstraint(self):
            return self.getTypedRuleContext(Pddl3Parser.SimpleDurationConstraintContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_simpleDurationConstraint

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterSimpleDurationConstraint(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitSimpleDurationConstraint(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitSimpleDurationConstraint(self)
            else:
                return visitor.visitChildren(self)




    def simpleDurationConstraint(self):

        localctx = Pddl3Parser.SimpleDurationConstraintContext(self, self._ctx, self.state)
        self.enterRule(localctx, 106, self.RULE_simpleDurationConstraint)
        try:
            self.state = 669
            la_ = self._interp.adaptivePredict(self._input,57,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 657
                self.match(Pddl3Parser.T__0)
                self.state = 658
                self.durOp()
                self.state = 659
                self.match(Pddl3Parser.T__49)
                self.state = 660
                self.durValue()
                self.state = 661
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 663
                self.match(Pddl3Parser.T__0)
                self.state = 664
                self.match(Pddl3Parser.T__30)
                self.state = 665
                self.timeSpecifier()
                self.state = 666
                self.simpleDurationConstraint()
                self.state = 667
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DurOpContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return Pddl3Parser.RULE_durOp

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDurOp(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDurOp(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDurOp(self)
            else:
                return visitor.visitChildren(self)




    def durOp(self):

        localctx = Pddl3Parser.DurOpContext(self, self._ctx, self.state)
        self.enterRule(localctx, 108, self.RULE_durOp)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 671
            _la = self._input.LA(1)
            if not((((_la) & ~0x3f) == 0 and ((1 << _la) & ((1 << Pddl3Parser.T__26) | (1 << Pddl3Parser.T__42) | (1 << Pddl3Parser.T__43))) != 0)):
                self._errHandler.recoverInline(self)
            else:
                self.consume()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DurValueContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NUMBER(self):
            return self.getToken(Pddl3Parser.NUMBER, 0)

        def fExp(self):
            return self.getTypedRuleContext(Pddl3Parser.FExpContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_durValue

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDurValue(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDurValue(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDurValue(self)
            else:
                return visitor.visitChildren(self)




    def durValue(self):

        localctx = Pddl3Parser.DurValueContext(self, self._ctx, self.state)
        self.enterRule(localctx, 110, self.RULE_durValue)
        try:
            self.state = 675
            la_ = self._interp.adaptivePredict(self._input,58,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 673
                self.match(Pddl3Parser.NUMBER)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 674
                self.fExp()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class DaEffectContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def daEffect(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.DaEffectContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.DaEffectContext,i)


        def timedEffect(self):
            return self.getTypedRuleContext(Pddl3Parser.TimedEffectContext,0)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def daGD(self):
            return self.getTypedRuleContext(Pddl3Parser.DaGDContext,0)


        def assignOp(self):
            return self.getTypedRuleContext(Pddl3Parser.AssignOpContext,0)


        def fHead(self):
            return self.getTypedRuleContext(Pddl3Parser.FHeadContext,0)


        def fExpDA(self):
            return self.getTypedRuleContext(Pddl3Parser.FExpDAContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_daEffect

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterDaEffect(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitDaEffect(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitDaEffect(self)
            else:
                return visitor.visitChildren(self)




    def daEffect(self):

        localctx = Pddl3Parser.DaEffectContext(self, self._ctx, self.state)
        self.enterRule(localctx, 112, self.RULE_daEffect)
        self._la = 0 # Token type
        try:
            self.state = 707
            la_ = self._interp.adaptivePredict(self._input,60,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 677
                self.match(Pddl3Parser.T__0)
                self.state = 678
                self.match(Pddl3Parser.T__20)
                self.state = 682
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.T__0:
                    self.state = 679
                    self.daEffect()
                    self.state = 684
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 685
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 686
                self.timedEffect()
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 687
                self.match(Pddl3Parser.T__0)
                self.state = 688
                self.match(Pddl3Parser.T__25)
                self.state = 689
                self.match(Pddl3Parser.T__0)
                self.state = 690
                self.typedVariableList()
                self.state = 691
                self.match(Pddl3Parser.T__2)
                self.state = 692
                self.daEffect()
                self.state = 693
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 4:
                self.enterOuterAlt(localctx, 4)
                self.state = 695
                self.match(Pddl3Parser.T__0)
                self.state = 696
                self.match(Pddl3Parser.T__36)
                self.state = 697
                self.daGD()
                self.state = 698
                self.timedEffect()
                self.state = 699
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 5:
                self.enterOuterAlt(localctx, 5)
                self.state = 701
                self.match(Pddl3Parser.T__0)
                self.state = 702
                self.assignOp()
                self.state = 703
                self.fHead()
                self.state = 704
                self.fExpDA()
                self.state = 705
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class TimedEffectContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def timeSpecifier(self):
            return self.getTypedRuleContext(Pddl3Parser.TimeSpecifierContext,0)


        def daEffect(self):
            return self.getTypedRuleContext(Pddl3Parser.DaEffectContext,0)


        def fAssignDA(self):
            return self.getTypedRuleContext(Pddl3Parser.FAssignDAContext,0)


        def assignOp(self):
            return self.getTypedRuleContext(Pddl3Parser.AssignOpContext,0)


        def fHead(self):
            return self.getTypedRuleContext(Pddl3Parser.FHeadContext,0)


        def fExp(self):
            return self.getTypedRuleContext(Pddl3Parser.FExpContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_timedEffect

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterTimedEffect(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitTimedEffect(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitTimedEffect(self)
            else:
                return visitor.visitChildren(self)




    def timedEffect(self):

        localctx = Pddl3Parser.TimedEffectContext(self, self._ctx, self.state)
        self.enterRule(localctx, 114, self.RULE_timedEffect)
        try:
            self.state = 727
            la_ = self._interp.adaptivePredict(self._input,61,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 709
                self.match(Pddl3Parser.T__0)
                self.state = 710
                self.match(Pddl3Parser.T__30)
                self.state = 711
                self.timeSpecifier()
                self.state = 712
                self.daEffect()
                self.state = 713
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 715
                self.match(Pddl3Parser.T__0)
                self.state = 716
                self.match(Pddl3Parser.T__30)
                self.state = 717
                self.timeSpecifier()
                self.state = 718
                self.fAssignDA()
                self.state = 719
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 721
                self.match(Pddl3Parser.T__0)
                self.state = 722
                self.assignOp()
                self.state = 723
                self.fHead()
                self.state = 724
                self.fExp()
                self.state = 725
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FAssignDAContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def assignOp(self):
            return self.getTypedRuleContext(Pddl3Parser.AssignOpContext,0)


        def fHead(self):
            return self.getTypedRuleContext(Pddl3Parser.FHeadContext,0)


        def fExpDA(self):
            return self.getTypedRuleContext(Pddl3Parser.FExpDAContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_fAssignDA

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFAssignDA(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFAssignDA(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFAssignDA(self)
            else:
                return visitor.visitChildren(self)




    def fAssignDA(self):

        localctx = Pddl3Parser.FAssignDAContext(self, self._ctx, self.state)
        self.enterRule(localctx, 116, self.RULE_fAssignDA)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 729
            self.match(Pddl3Parser.T__0)
            self.state = 730
            self.assignOp()
            self.state = 731
            self.fHead()
            self.state = 732
            self.fExpDA()
            self.state = 733
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class FExpDAContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def binaryOp(self):
            return self.getTypedRuleContext(Pddl3Parser.BinaryOpContext,0)


        def fExpDA(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.FExpDAContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.FExpDAContext,i)


        def fExp(self):
            return self.getTypedRuleContext(Pddl3Parser.FExpContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_fExpDA

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterFExpDA(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitFExpDA(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitFExpDA(self)
            else:
                return visitor.visitChildren(self)




    def fExpDA(self):

        localctx = Pddl3Parser.FExpDAContext(self, self._ctx, self.state)
        self.enterRule(localctx, 118, self.RULE_fExpDA)
        try:
            self.state = 748
            la_ = self._interp.adaptivePredict(self._input,63,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 735
                self.match(Pddl3Parser.T__0)
                self.state = 742
                la_ = self._interp.adaptivePredict(self._input,62,self._ctx)
                if la_ == 1:
                    self.state = 736
                    self.binaryOp()
                    self.state = 737
                    self.fExpDA()
                    self.state = 738
                    self.fExpDA()
                    pass

                elif la_ == 2:
                    self.state = 740
                    self.match(Pddl3Parser.T__7)
                    self.state = 741
                    self.fExpDA()
                    pass


                self.state = 744
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 746
                self.match(Pddl3Parser.T__49)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 747
                self.fExp()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ProblemContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def problemDecl(self):
            return self.getTypedRuleContext(Pddl3Parser.ProblemDeclContext,0)


        def problemDomain(self):
            return self.getTypedRuleContext(Pddl3Parser.ProblemDomainContext,0)


        def init(self):
            return self.getTypedRuleContext(Pddl3Parser.InitContext,0)


        def goal(self):
            return self.getTypedRuleContext(Pddl3Parser.GoalContext,0)


        def requireDef(self):
            return self.getTypedRuleContext(Pddl3Parser.RequireDefContext,0)


        def objectDecl(self):
            return self.getTypedRuleContext(Pddl3Parser.ObjectDeclContext,0)


        def probConstraints(self):
            return self.getTypedRuleContext(Pddl3Parser.ProbConstraintsContext,0)


        def metricSpec(self):
            return self.getTypedRuleContext(Pddl3Parser.MetricSpecContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_problem

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterProblem(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitProblem(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitProblem(self)
            else:
                return visitor.visitChildren(self)




    def problem(self):

        localctx = Pddl3Parser.ProblemContext(self, self._ctx, self.state)
        self.enterRule(localctx, 120, self.RULE_problem)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 750
            self.match(Pddl3Parser.T__0)
            self.state = 751
            self.match(Pddl3Parser.T__1)
            self.state = 752
            self.problemDecl()
            self.state = 753
            self.problemDomain()
            self.state = 755
            la_ = self._interp.adaptivePredict(self._input,64,self._ctx)
            if la_ == 1:
                self.state = 754
                self.requireDef()


            self.state = 758
            la_ = self._interp.adaptivePredict(self._input,65,self._ctx)
            if la_ == 1:
                self.state = 757
                self.objectDecl()


            self.state = 760
            self.init()
            self.state = 761
            self.goal()
            self.state = 763
            la_ = self._interp.adaptivePredict(self._input,66,self._ctx)
            if la_ == 1:
                self.state = 762
                self.probConstraints()


            self.state = 766
            _la = self._input.LA(1)
            if _la==Pddl3Parser.T__0:
                self.state = 765
                self.metricSpec()


            self.state = 768
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ProblemDeclContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_problemDecl

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterProblemDecl(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitProblemDecl(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitProblemDecl(self)
            else:
                return visitor.visitChildren(self)




    def problemDecl(self):

        localctx = Pddl3Parser.ProblemDeclContext(self, self._ctx, self.state)
        self.enterRule(localctx, 122, self.RULE_problemDecl)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 770
            self.match(Pddl3Parser.T__0)
            self.state = 771
            self.match(Pddl3Parser.T__50)
            self.state = 772
            self.match(Pddl3Parser.NAME)
            self.state = 773
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ProblemDomainContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_problemDomain

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterProblemDomain(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitProblemDomain(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitProblemDomain(self)
            else:
                return visitor.visitChildren(self)




    def problemDomain(self):

        localctx = Pddl3Parser.ProblemDomainContext(self, self._ctx, self.state)
        self.enterRule(localctx, 124, self.RULE_problemDomain)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 775
            self.match(Pddl3Parser.T__0)
            self.state = 776
            self.match(Pddl3Parser.T__51)
            self.state = 777
            self.match(Pddl3Parser.NAME)
            self.state = 778
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ObjectDeclContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def typedNameList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedNameListContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_objectDecl

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterObjectDecl(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitObjectDecl(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitObjectDecl(self)
            else:
                return visitor.visitChildren(self)




    def objectDecl(self):

        localctx = Pddl3Parser.ObjectDeclContext(self, self._ctx, self.state)
        self.enterRule(localctx, 126, self.RULE_objectDecl)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 780
            self.match(Pddl3Parser.T__0)
            self.state = 781
            self.match(Pddl3Parser.T__52)
            self.state = 782
            self.typedNameList()
            self.state = 783
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class InitContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def initEl(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.InitElContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.InitElContext,i)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_init

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterInit(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitInit(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitInit(self)
            else:
                return visitor.visitChildren(self)




    def init(self):

        localctx = Pddl3Parser.InitContext(self, self._ctx, self.state)
        self.enterRule(localctx, 128, self.RULE_init)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 785
            self.match(Pddl3Parser.T__0)
            self.state = 786
            self.match(Pddl3Parser.T__53)
            self.state = 790
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==Pddl3Parser.T__0:
                self.state = 787
                self.initEl()
                self.state = 792
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 793
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class InitElContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def nameLiteral(self):
            return self.getTypedRuleContext(Pddl3Parser.NameLiteralContext,0)


        def fHead(self):
            return self.getTypedRuleContext(Pddl3Parser.FHeadContext,0)


        def NUMBER(self):
            return self.getToken(Pddl3Parser.NUMBER, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_initEl

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterInitEl(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitInitEl(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitInitEl(self)
            else:
                return visitor.visitChildren(self)




    def initEl(self):

        localctx = Pddl3Parser.InitElContext(self, self._ctx, self.state)
        self.enterRule(localctx, 130, self.RULE_initEl)
        try:
            self.state = 808
            la_ = self._interp.adaptivePredict(self._input,69,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 795
                self.nameLiteral()
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 796
                self.match(Pddl3Parser.T__0)
                self.state = 797
                self.match(Pddl3Parser.T__26)
                self.state = 798
                self.fHead()
                self.state = 799
                self.match(Pddl3Parser.NUMBER)
                self.state = 800
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 802
                self.match(Pddl3Parser.T__0)
                self.state = 803
                self.match(Pddl3Parser.T__30)
                self.state = 804
                self.match(Pddl3Parser.NUMBER)
                self.state = 805
                self.nameLiteral()
                self.state = 806
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class NameLiteralContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def atomicNameFormula(self):
            return self.getTypedRuleContext(Pddl3Parser.AtomicNameFormulaContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_nameLiteral

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterNameLiteral(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitNameLiteral(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitNameLiteral(self)
            else:
                return visitor.visitChildren(self)




    def nameLiteral(self):

        localctx = Pddl3Parser.NameLiteralContext(self, self._ctx, self.state)
        self.enterRule(localctx, 132, self.RULE_nameLiteral)
        try:
            self.state = 816
            la_ = self._interp.adaptivePredict(self._input,70,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 810
                self.atomicNameFormula()
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 811
                self.match(Pddl3Parser.T__0)
                self.state = 812
                self.match(Pddl3Parser.T__22)
                self.state = 813
                self.atomicNameFormula()
                self.state = 814
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class AtomicNameFormulaContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def predicate(self):
            return self.getTypedRuleContext(Pddl3Parser.PredicateContext,0)


        def NAME(self, i:int=None):
            if i is None:
                return self.getTokens(Pddl3Parser.NAME)
            else:
                return self.getToken(Pddl3Parser.NAME, i)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_atomicNameFormula

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterAtomicNameFormula(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitAtomicNameFormula(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitAtomicNameFormula(self)
            else:
                return visitor.visitChildren(self)




    def atomicNameFormula(self):

        localctx = Pddl3Parser.AtomicNameFormulaContext(self, self._ctx, self.state)
        self.enterRule(localctx, 134, self.RULE_atomicNameFormula)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 818
            self.match(Pddl3Parser.T__0)
            self.state = 819
            self.predicate()
            self.state = 823
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==Pddl3Parser.NAME:
                self.state = 820
                self.match(Pddl3Parser.NAME)
                self.state = 825
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 826
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class GoalContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def goalDesc(self):
            return self.getTypedRuleContext(Pddl3Parser.GoalDescContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_goal

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterGoal(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitGoal(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitGoal(self)
            else:
                return visitor.visitChildren(self)




    def goal(self):

        localctx = Pddl3Parser.GoalContext(self, self._ctx, self.state)
        self.enterRule(localctx, 136, self.RULE_goal)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 828
            self.match(Pddl3Parser.T__0)
            self.state = 829
            self.match(Pddl3Parser.T__54)
            self.state = 830
            self.goalDesc()
            self.state = 831
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ProbConstraintsContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def prefConGD(self):
            return self.getTypedRuleContext(Pddl3Parser.PrefConGDContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_probConstraints

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterProbConstraints(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitProbConstraints(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitProbConstraints(self)
            else:
                return visitor.visitChildren(self)




    def probConstraints(self):

        localctx = Pddl3Parser.ProbConstraintsContext(self, self._ctx, self.state)
        self.enterRule(localctx, 138, self.RULE_probConstraints)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 833
            self.match(Pddl3Parser.T__0)
            self.state = 834
            self.match(Pddl3Parser.T__13)
            self.state = 835
            self.prefConGD()
            self.state = 836
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class PrefConGDContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def prefConGD(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.PrefConGDContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.PrefConGDContext,i)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def conGD(self):
            return self.getTypedRuleContext(Pddl3Parser.ConGDContext,0)


        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_prefConGD

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterPrefConGD(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitPrefConGD(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitPrefConGD(self)
            else:
                return visitor.visitChildren(self)




    def prefConGD(self):

        localctx = Pddl3Parser.PrefConGDContext(self, self._ctx, self.state)
        self.enterRule(localctx, 140, self.RULE_prefConGD)
        self._la = 0 # Token type
        try:
            self.state = 864
            la_ = self._interp.adaptivePredict(self._input,74,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 838
                self.match(Pddl3Parser.T__0)
                self.state = 839
                self.match(Pddl3Parser.T__20)
                self.state = 843
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.T__0:
                    self.state = 840
                    self.prefConGD()
                    self.state = 845
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 846
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 847
                self.match(Pddl3Parser.T__0)
                self.state = 848
                self.match(Pddl3Parser.T__25)
                self.state = 849
                self.match(Pddl3Parser.T__0)
                self.state = 850
                self.typedVariableList()
                self.state = 851
                self.match(Pddl3Parser.T__2)
                self.state = 852
                self.prefConGD()
                self.state = 853
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 855
                self.match(Pddl3Parser.T__0)
                self.state = 856
                self.match(Pddl3Parser.T__29)
                self.state = 858
                _la = self._input.LA(1)
                if _la==Pddl3Parser.NAME:
                    self.state = 857
                    self.match(Pddl3Parser.NAME)


                self.state = 860
                self.conGD()
                self.state = 861
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 4:
                self.enterOuterAlt(localctx, 4)
                self.state = 863
                self.conGD()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class MetricSpecContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def optimization(self):
            return self.getTypedRuleContext(Pddl3Parser.OptimizationContext,0)


        def metricFExp(self):
            return self.getTypedRuleContext(Pddl3Parser.MetricFExpContext,0)


        def getRuleIndex(self):
            return Pddl3Parser.RULE_metricSpec

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterMetricSpec(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitMetricSpec(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitMetricSpec(self)
            else:
                return visitor.visitChildren(self)




    def metricSpec(self):

        localctx = Pddl3Parser.MetricSpecContext(self, self._ctx, self.state)
        self.enterRule(localctx, 142, self.RULE_metricSpec)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 866
            self.match(Pddl3Parser.T__0)
            self.state = 867
            self.match(Pddl3Parser.T__55)
            self.state = 868
            self.optimization()
            self.state = 869
            self.metricFExp()
            self.state = 870
            self.match(Pddl3Parser.T__2)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class OptimizationContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return Pddl3Parser.RULE_optimization

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterOptimization(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitOptimization(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitOptimization(self)
            else:
                return visitor.visitChildren(self)




    def optimization(self):

        localctx = Pddl3Parser.OptimizationContext(self, self._ctx, self.state)
        self.enterRule(localctx, 144, self.RULE_optimization)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 872
            _la = self._input.LA(1)
            if not(_la==Pddl3Parser.T__56 or _la==Pddl3Parser.T__57):
                self._errHandler.recoverInline(self)
            else:
                self.consume()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class MetricFExpContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def binaryOp(self):
            return self.getTypedRuleContext(Pddl3Parser.BinaryOpContext,0)


        def metricFExp(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.MetricFExpContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.MetricFExpContext,i)


        def multiOp(self):
            return self.getTypedRuleContext(Pddl3Parser.MultiOpContext,0)


        def NUMBER(self):
            return self.getToken(Pddl3Parser.NUMBER, 0)

        def fHead(self):
            return self.getTypedRuleContext(Pddl3Parser.FHeadContext,0)


        def NAME(self):
            return self.getToken(Pddl3Parser.NAME, 0)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_metricFExp

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterMetricFExp(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitMetricFExp(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitMetricFExp(self)
            else:
                return visitor.visitChildren(self)




    def metricFExp(self):

        localctx = Pddl3Parser.MetricFExpContext(self, self._ctx, self.state)
        self.enterRule(localctx, 146, self.RULE_metricFExp)
        self._la = 0 # Token type
        try:
            self.state = 901
            la_ = self._interp.adaptivePredict(self._input,76,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 874
                self.match(Pddl3Parser.T__0)
                self.state = 875
                self.binaryOp()
                self.state = 876
                self.metricFExp()
                self.state = 877
                self.metricFExp()
                self.state = 878
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 880
                self.match(Pddl3Parser.T__0)
                self.state = 881
                self.multiOp()
                self.state = 882
                self.metricFExp()
                self.state = 884 
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while True:
                    self.state = 883
                    self.metricFExp()
                    self.state = 886 
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)
                    if not (_la==Pddl3Parser.T__0 or _la==Pddl3Parser.NAME or _la==Pddl3Parser.NUMBER):
                        break

                self.state = 888
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 890
                self.match(Pddl3Parser.T__0)
                self.state = 891
                self.match(Pddl3Parser.T__7)
                self.state = 892
                self.metricFExp()
                self.state = 893
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 4:
                self.enterOuterAlt(localctx, 4)
                self.state = 895
                self.match(Pddl3Parser.NUMBER)
                pass

            elif la_ == 5:
                self.enterOuterAlt(localctx, 5)
                self.state = 896
                self.fHead()
                pass

            elif la_ == 6:
                self.enterOuterAlt(localctx, 6)
                self.state = 897
                self.match(Pddl3Parser.T__0)
                self.state = 898
                self.match(Pddl3Parser.T__58)
                self.state = 899
                self.match(Pddl3Parser.NAME)
                self.state = 900
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ConGDContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser

        def conGD(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.ConGDContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.ConGDContext,i)


        def typedVariableList(self):
            return self.getTypedRuleContext(Pddl3Parser.TypedVariableListContext,0)


        def goalDesc(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(Pddl3Parser.GoalDescContext)
            else:
                return self.getTypedRuleContext(Pddl3Parser.GoalDescContext,i)


        def NUMBER(self, i:int=None):
            if i is None:
                return self.getTokens(Pddl3Parser.NUMBER)
            else:
                return self.getToken(Pddl3Parser.NUMBER, i)

        def getRuleIndex(self):
            return Pddl3Parser.RULE_conGD

        def enterRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.enterConGD(self)

        def exitRule(self, listener:ParseTreeListener):
            if isinstance( listener, Pddl3Listener ):
                listener.exitConGD(self)

        def accept(self, visitor:ParseTreeVisitor):
            if isinstance( visitor, Pddl3Visitor ):
                return visitor.visitConGD(self)
            else:
                return visitor.visitChildren(self)




    def conGD(self):

        localctx = Pddl3Parser.ConGDContext(self, self._ctx, self.state)
        self.enterRule(localctx, 148, self.RULE_conGD)
        self._la = 0 # Token type
        try:
            self.state = 979
            la_ = self._interp.adaptivePredict(self._input,78,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 903
                self.match(Pddl3Parser.T__0)
                self.state = 904
                self.match(Pddl3Parser.T__20)
                self.state = 908
                self._errHandler.sync(self)
                _la = self._input.LA(1)
                while _la==Pddl3Parser.T__0:
                    self.state = 905
                    self.conGD()
                    self.state = 910
                    self._errHandler.sync(self)
                    _la = self._input.LA(1)

                self.state = 911
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 912
                self.match(Pddl3Parser.T__0)
                self.state = 913
                self.match(Pddl3Parser.T__25)
                self.state = 914
                self.match(Pddl3Parser.T__0)
                self.state = 915
                self.typedVariableList()
                self.state = 916
                self.match(Pddl3Parser.T__2)
                self.state = 917
                self.conGD()
                self.state = 918
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 3:
                self.enterOuterAlt(localctx, 3)
                self.state = 920
                self.match(Pddl3Parser.T__0)
                self.state = 921
                self.match(Pddl3Parser.T__30)
                self.state = 922
                self.match(Pddl3Parser.T__33)
                self.state = 923
                self.goalDesc()
                self.state = 924
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 4:
                self.enterOuterAlt(localctx, 4)
                self.state = 926
                self.match(Pddl3Parser.T__0)
                self.state = 927
                self.match(Pddl3Parser.T__59)
                self.state = 928
                self.goalDesc()
                self.state = 929
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 5:
                self.enterOuterAlt(localctx, 5)
                self.state = 931
                self.match(Pddl3Parser.T__0)
                self.state = 932
                self.match(Pddl3Parser.T__60)
                self.state = 933
                self.goalDesc()
                self.state = 934
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 6:
                self.enterOuterAlt(localctx, 6)
                self.state = 936
                self.match(Pddl3Parser.T__0)
                self.state = 937
                self.match(Pddl3Parser.T__61)
                self.state = 938
                self.match(Pddl3Parser.NUMBER)
                self.state = 939
                self.goalDesc()
                self.state = 940
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 7:
                self.enterOuterAlt(localctx, 7)
                self.state = 942
                self.match(Pddl3Parser.T__0)
                self.state = 943
                self.match(Pddl3Parser.T__62)
                self.state = 944
                self.goalDesc()
                self.state = 945
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 8:
                self.enterOuterAlt(localctx, 8)
                self.state = 947
                self.match(Pddl3Parser.T__0)
                self.state = 948
                self.match(Pddl3Parser.T__63)
                self.state = 949
                self.goalDesc()
                self.state = 950
                self.goalDesc()
                self.state = 951
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 9:
                self.enterOuterAlt(localctx, 9)
                self.state = 953
                self.match(Pddl3Parser.T__0)
                self.state = 954
                self.match(Pddl3Parser.T__64)
                self.state = 955
                self.goalDesc()
                self.state = 956
                self.goalDesc()
                self.state = 957
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 10:
                self.enterOuterAlt(localctx, 10)
                self.state = 959
                self.match(Pddl3Parser.T__0)
                self.state = 960
                self.match(Pddl3Parser.T__65)
                self.state = 961
                self.match(Pddl3Parser.NUMBER)
                self.state = 962
                self.goalDesc()
                self.state = 963
                self.goalDesc()
                self.state = 964
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 11:
                self.enterOuterAlt(localctx, 11)
                self.state = 966
                self.match(Pddl3Parser.T__0)
                self.state = 967
                self.match(Pddl3Parser.T__66)
                self.state = 968
                self.match(Pddl3Parser.NUMBER)
                self.state = 969
                self.match(Pddl3Parser.NUMBER)
                self.state = 970
                self.goalDesc()
                self.state = 971
                self.match(Pddl3Parser.T__2)
                pass

            elif la_ == 12:
                self.enterOuterAlt(localctx, 12)
                self.state = 973
                self.match(Pddl3Parser.T__0)
                self.state = 974
                self.match(Pddl3Parser.T__67)
                self.state = 975
                self.match(Pddl3Parser.NUMBER)
                self.state = 976
                self.goalDesc()
                self.state = 977
                self.match(Pddl3Parser.T__2)
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx




