grammar Dosato;

INTEGER: [0-9]+;         // Matches integers (e.g., 123, 456)
DOUBLE: [0-9]+ '.' [0-9]+; // Matches floats (e.g., 12.34, 0.5)
FLOAT: DOUBLE | INTEGER 'e' [0-9]+; // Matches floats with exponent (e.g., 1e3, 2.5e-1)
FLOAT_LIT: FLOAT'F'; // Matches floats with 'F' suffix (e.g., 1.0F, 2.5e-1F)
BASE2_INTEGER: '0b'[01]+; // Matches binary integers (e.g., 0b1010, 0b1111)
BASE8_INTEGER: '0o'[0-7]+; // Matches octal integers (e.g., 0o123, 0o456)
BASE8_INTEGER_ALT: '0'[0-7]+; // Matches octal integers (e.g., 0123, 0456)
BASE16_INTEGER: '0x'[0-9a-fA-F]+; // Matches hexadecimal integers (e.g., 0x1A, 0xFF)

INFINITY: 'Infinity';
NAN: 'NaN';

STRING
    : '"' (ESC | SAFECODEPOINT)* '"' // Match valid strings
    ;

UNCLOSED_STRING
    : '"' (ESC | SAFECODEPOINT)* -> channel(HIDDEN)
    ;

CHAR
    : '\'' (ESC | SAFECODEPOINT)* '\''
    ;

TEMPLATE_STRING
    : '`' (TEMPLATE_ESC | TEMPLATE_SAFE)* '`'
    ;

fragment TEMPLATE_SAFE
    : ~[`\\${}]+ // Safe characters that do not conflict with template syntax
    ;

fragment TEMPLATE_ESC
    : '\\' (["\\/bfnrt`$] | UNICODE) // Escape sequences for template strings
    | '{' .*? '}' // Template expressions
    ;

fragment SAFECODEPOINT
    : ~["\\\r\n]
    ;

fragment ESC
    : '\\' (["\\/bfnrt] | UNICODE)
    ;

fragment UNICODE
    : 'u' HEX HEX HEX HEX
    ;

fragment HEX
    : [0-9a-fA-F]
    ;

// Whitespace and comments
WS: [ \t\r\n]+ -> skip; // Skip whitespace
COMMENT: '//' ~[\r\n]* -> skip; // Skip comments
BLOCK_COMMENT: '/*' .*? '*/' -> skip; // Skip block comments

OPERATOR
    : '+' 
    | '-'
    | '*' 
    | '/' 
    | '%' 
    | '>' 
    | '<' 
    | '&' 
    | '^' 
    | '->' 
    | '#' 
    | '==' 
    | '!=' 
    | '>=' 
    | '<=' 
    | '&&' 
    | '||' 
    | '<<' 
    | '>>' 
    | '**' 
    | '>|' 
    | '<|' 
    | ':>' 
    | ':<' 
    | ':>=' 
    | ':<=' 
    | '??' 
    | '?->' 
    | '|>' 
    | '===' 
    | '!=='
    | '!' 
    | '~'
    | '^/'
    | '!-'
    ;

BINARY_ASSIGN_OPERATOR
    : '='
    | '+='
    | '-='
    | '*='
    | '/='
    | '%='
    | '&='
    | '|='
    | '^='
    | '>>='
    | '<<='
    | '**='
    | '>|='
    | '<|='
    | '??='
    | '^/='
    | '|>='
    ;

INCREMENT_OPERATOR
    : '++'
    | '--'
    ;

OPERATOR_ARROW: '=>';

COMMA: ',';

TYPE
    : 'int'
    | 'bool'
    | 'string'
    | 'float'
    | 'double'
    | 'char'
    | 'short'
    | 'long'
    | 'byte'
    | 'void'
    | 'array'
    | 'uint'
    | 'ushort'
    | 'ulong'
    | 'ubyte'
    | 'object'
    | 'var'
    | 'function'
    ;

BOOLEAN
    : 'true'
    | 'false'
    ;

NULL: 'null';

DO: 'do';
MAKE: 'make';
CONST: 'const';
SET: 'set';
DEFINE: 'define';
IMPLEMENT: 'implement';
RETURN: 'return';
BREAK: 'break';
CONTINUE: 'continue';
CLASS: 'class';
INHERIT: 'inherit';
IMPORT: 'import';
INCLUDE: 'include';
ENUM: 'enum';
IF: 'if';
SWITCH: 'switch';
MATCH: 'match';
THEN: 'then';
ELSE: 'else';
WHILE: 'while';
UNTIL: 'until';
FOR: 'for';
CATCH: 'catch';
WHEN: 'when';
UNLESS: 'unless';
OTHER: 'other';
IN: 'in';

BRACKET_ROUND_OPEN: '(';
BRACKET_ROUND_CLOSE: ')';
BRACKET_SQUARE_OPEN: '[';
BRACKET_SQUARE_CLOSE: ']';
BRACKET_CURLY_OPEN: '{';
BRACKET_CURLY_CLOSE: '}';

COLON: ':';
QUESTION_MARK: '?';

IDENTIFIER: [a-zA-Z_] [a-zA-Z_0-9]*; // Matches identifiers (e.g., foo, bar, baz)


// Entry point for parsing
program: master_statement+ EOF;

// Literals
number
    : INTEGER 
    | DOUBLE 
    | FLOAT 
    | BASE2_INTEGER 
    | BASE8_INTEGER 
    | BASE8_INTEGER_ALT 
    | BASE16_INTEGER
    | INFINITY
    | NAN
    ;

string_literal
    : STRING
    | CHAR
    | TEMPLATE_STRING
    ;

array_expression: BRACKET_SQUARE_OPEN (expression (COMMA expression)*)? BRACKET_SQUARE_CLOSE;
object_expression: BRACKET_CURLY_OPEN (expression COLON expression (COMMA expression COLON expression)*)? BRACKET_CURLY_CLOSE;

function_call: expression BRACKET_ROUND_OPEN (expression (COMMA expression)*)? BRACKET_ROUND_CLOSE;

tuple_expression: expression (COMMA expression)*;
tuple_assign_expression: IDENTIFIER (COMMA IDENTIFIER)*;

function_declaration_arguments: BRACKET_ROUND_OPEN (TYPE? IDENTIFIER (COMMA TYPE? IDENTIFIER)*)? BRACKET_ROUND_CLOSE;

type_cast: BRACKET_ROUND_OPEN TYPE BRACKET_ROUND_CLOSE;

lambda_expression: TYPE function_declaration_arguments OPERATOR_ARROW block;

expression
    : BRACKET_ROUND_OPEN expression BRACKET_ROUND_CLOSE
    | expression OPERATOR expression
    | OPERATOR expression
    | type_cast expression
    | expression QUESTION_MARK expression COLON expression
    | expression BRACKET_ROUND_OPEN (expression (COMMA expression)*)? BRACKET_ROUND_CLOSE
    | array_expression
    | object_expression
    | lambda_expression
    | number
    | string_literal
    | IDENTIFIER
    | BOOLEAN
    | NULL
    ;

master_statement
    : DO call_block master_extension?
    | (MAKE | CONST) TYPE? tuple_assign_expression BINARY_ASSIGN_OPERATOR tuple_expression
    | SET tuple_expression (BINARY_ASSIGN_OPERATOR (expression | tuple_expression) | INCREMENT_OPERATOR) master_extension?
    | (DEFINE | IMPLEMENT) TYPE? IDENTIFIER function_declaration_arguments block
    | RETURN expression master_extension?
    | BREAK master_extension?
    | CONTINUE master_extension?
    | CLASS IDENTIFIER function_declaration_arguments block
    | INHERIT expression
    | IMPORT STRING
    | INCLUDE STRING
    | ENUM IDENTIFIER BRACKET_CURLY_OPEN (IDENTIFIER (COMMA IDENTIFIER)*)? BRACKET_CURLY_CLOSE
    | IF expression THEN call_block else_block? master_extension?
    | (SWITCH | MATCH) expression OPERATOR_ARROW switch_block master_extension?
    ;

master_extension
    : (WHEN | UNLESS) expression else_block?
    | (WHILE | UNTIL) expression
    | FOR (expression IN)? expression
    | CATCH call_block
    | THEN call_block
    ;

call_block: block | function_call;

else_block: (ELSE call_block | ELSE IF expression THEN call_block);

block: BRACKET_CURLY_OPEN master_statement* BRACKET_CURLY_CLOSE;

case: (expression | OTHER);
switch_block: BRACKET_CURLY_OPEN (case (COMMA case)* OPERATOR_ARROW block)* BRACKET_CURLY_CLOSE;