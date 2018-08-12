## Snack design

- Snack design is highly mixed between C# and Python. The language is extremely similar to Python instead it's more
static typed. The code than is compiled to SIR, which is similar to C# compiled to CIL.

## Variable declaration
- Variable declaration is done using *var*. A variable can be a string, number or an object reference. 
- For example:
    **var a = 15** or **var b = 'hi'** or **var c= new array(15, 12, 24 ,2242, 123)**

## Method declaration
- **fn <func_name>(arg): **
- Every method or block requires identation. Identation only counted on logical line.

## Common statement
- if a: <block> else: <block>
- for <init_jobs>; <cont_conditions>; <end_jobs>: <block>
- while <cont_conditions>: <block>
- do: <block> unless <condition>:

## Limitation of Language
- No number type really specified. However, you can still do binary operation as usual. Snack will
automaticlly cast number for you
- OOP is possible but not yet

## Limitation of SIR
- SIR like CIL, brings all scope variables to method variables. This will have a big impact on massive script,
since there are only 300 local variables allowed.