@@
expression timer_variable, function_name, data_expression;
@@
- init_timer(&timer_variable);
- timer_variable.function = function_name;
- timer_variable.data = data_expression;
+ setup_timer(&timer_variable, function_name, data_expression);


