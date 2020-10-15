# Zest Spec
[BNF Validator](https://bnfplayground.pauliankline.com/)
# Casting
## Implicit
## Explicit
# Functions
```
<param_definition> ::= <type> <ws> <identifier>
<param_list> ::= "(" <ws>? ((<param_definition> "," <ws>)* <param_definition>)? <ws>? ")"
<return_signature> ::= "->" <ws> <type>
<function_signature> ::= "fn" <ws> <identifier> <param_signature> (<ws> <return_signature>)?
<function_definition> ::= <function_signature> <ws>? <block>
```
Requirements: TODO
# Statements
```
<statement> ::= <return> | <variable_definition> | <assignment> | <if>
```
## Return
```
<return> ::= "return" <ws> <exp>
```
Requirements:
- The value returned must be the same type as (or implicitly casted to) the return type of the parent function.

## Variable Definition
```
<variable_definition> ::= ("let" | "const") <ws> <identifier> <ws> <type>
```
Requirements:
- The name of the variable must not already be registered in the current scope.

## Variable Assignment
```
<assignment> ::= <identifier> <ws> "=" <ws> <exp>
```
Requirements:
- The variable must be defined in scope.
- The value being assigned must be the same type as (or implicitly casted to) the type of the variable.
- The variable must not be const.
## If Statement
```
<if> ::= "if" <ws> <exp> <ws> <block> (<ws> (<else> | <elif>))?
<elif> ::= "elif" <ws> <exp> <ws> <block> (<ws> (<else> | <elif>))?
<else> ::= "else" <ws> <block>
```
Requirements:
- Conditions for if and elif must be castable to bool.
# Expressions
```
<exp> ::= <int_literal> | <reference>
```
## Reference
```
<reference> ::= <identifier>
```
Requirements:
- The variable must be defined in scope.
## Integer Literal
```
<int_literal> ::= "-"? <digit>+
```
Requirements: TODO
# Full EBNF
```
<program> ::= (<ws>? <function_definition> <ws>?)+

/* misc */
<ws> ::= (" " | "\t" | "\n")+ 
<letter> ::= ([a-z] | [A-Z])
<digit> ::= [0-9]
<identifier> ::= (<letter> | "_") (<letter> | "_" | <digit>)*
<type> ::= <identifier>
/* TODO: proper line terminal logic */
<block> ::= "{" (<ws> | <statement>)* "}"

/* functions */
<param_definition> ::= <type> <ws> <identifier>
<param_list> ::= "(" <ws>? ((<param_definition> "," <ws>)* <param_definition>)? <ws>? ")"
<return_signature> ::= "->" <ws> <type>
<function_signature> ::= "fn" <ws> <identifier> <param_signature> (<ws> <return_signature>)?
<function_definition> ::= <function_signature> <ws>? <block>

/* statements */
<statement> ::= <return> | <variable_definition> | <assignment> | <if>

<if> ::= "if" <ws> <exp> <ws> <block> (<ws> (<else> | <elif>))?
<elif> ::= "elif" <ws> <exp> <ws> <block> (<ws> (<else> | <elif>))?
<else> ::= "else" <ws> <block>

<variable_definition> ::= ("let" | "const") <ws> <identifier> <ws> <type>
<assignment> ::= <identifier> <ws> "=" <ws> <exp>

<return> ::= "return" <ws> <exp>

/* expressions */
<exp> ::= <int_literal> | <reference>
<reference> ::= <identifier>
<int_literal> ::= "-"? <digit>+
```
