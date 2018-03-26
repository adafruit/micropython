try:
    import array
except ImportError:
    print("SKIP")
    raise SystemExit

def raises(f):
    try:
        f()
    except:
        print(True)
    else:
        print(False)

def f():
    a = array.array('O')
    a[:] = array.array('P', [4])
    print(a)
raises(f)

def f():
    a = array.array('b')
    a[:] = array.array('B')
    print(a)
raises(f)

def f():
    a = array.array('O')
    a = a + 'bbbbbbbb'
    print(a)
raises(f)

def f():
    a = array.array('O')
    a = a + array.array('P', [4])
    print(a)
raises(f)

def f():
    a = array.array('O')
    a += array.array('P', [4])
    print(a)
raises(f)

def f():
    a = array.array('O')
    a.extend('bbbbbbbb')
    print(a)
raises(f)

def f():
    a = array.array('O')
    a.extend(array.array('P', [4]))
    print(a)
raises(f)

def f():
    a = array.array('O', bytearray('bbbbbbbb'))
    print(a)
raises(f)

def f():
    a = array.array('O', bytearray('bbbbbbbb'))
