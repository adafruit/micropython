
def this_function_definition_crashes_because_await_is_not_allowed_unless_the_function_is_async():
    yield 'a'             # Does not make it async
    yield from (1, 2, 3)  # Does not make it async
    await 'hello'         # Does not make it async
