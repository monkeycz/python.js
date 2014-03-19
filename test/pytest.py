
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
		'kkk': __import__('os')
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
		print 'py_test -> ', e
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
