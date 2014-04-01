
def test_01(value):
	print value
	return value * 2

def test_02(value):
	print value
	return value * 1.5

def test_03(value):
	print value
	return value + ', hello python'

def test_04(value):
	print value
	return not value

def test_05(value):
	print value
	if value is None:
		return None
	else:
		raise Exception()

def test_06(value):
	print value
	return value + [4, 5, 6]

def test_07(value):
	print value
	value.update({'ccc': 333, 'ddd': 444})
	return value

def test_08(value):
	print value
	return {
		'aaa': 111, 222: 'bbb', 'ccc': True, 'ddd': 33.3, 
		'eee': lambda x,y: x+y, 
		'fff': [444, '555', 666], 
		'ggg': {'hhh': 777, 888: 'iii', 'jjj': lambda x,y: x*y},
		# 'kkk': __import__('os')
	}

def test_09(js_func):
	def py_func_02(caller, x, y):
		print caller, ' -> py_func_02'
		return x * y
	def py_func_01(caller, x, y):
		print caller, ' -> py_func_01'
		return js_func('py_func_01', x, y, py_func_02)
	return py_func_01

def test_10(js_func):
	try:
		js_func()
	except Exception as e:
		print 'py_test -> ', e, type(e)
		raise Exception('py_test_exception')

class Test_11(object):
	def __init__(self):
		print 'Test_11 -> __init__'
		self.aaa = 11

	def get_aaa(self):
		print 'Test_11 -> get_aaa: ', self.aaa
		return self.aaa

	def get_bbb(self):
		print 'Test_11 -> get_bbb: ', self.bbb
		return self.bbb

test_12 = [1, 2, 3]

test_13 = {'aaa': 111, 'bbb': 222}

def test_14(x):
	return [x] * 3

def test_15(js_func):
	return js_func()
test_15.data = {'aaa': 111, 'bbb': 222}

def test_16(make_error=True):
	print 'test_16 in'
	if make_error:
		import time
		time.sleep(2)
	print 'test_16 out'
	if make_error:
		raise SyntaxError('async_exception')
	return 'hello async'
def async_cb(error, result):
	print 'py async error: ', error, type(error)
	print 'py async result: ', result
test_16.async_cb = async_cb

def test_17():
	def aaa(): pass
	def bbb(): pass
	return [aaa, bbb]
