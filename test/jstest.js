
var python;
try {
	python = require('../index.js');
} catch (e) {
	python = require('node-python');
}

var sys = python.import('sys');
var os = python.import('os');

if (typeof(__dirname) != 'undefined')
	sys.path.append(__dirname);
else
	sys.path.append(os.path.join(os.getcwd(), 'node_modules/node-python/test'));

var pytest = python.import('pytest');

function test() {
	console.log(pytest.test_01(5));

	console.log(pytest.test_02(7.5));

	console.log(pytest.test_03('hello javascript'));

	console.log(pytest.test_04(true), pytest.test_04(false));

	console.log(pytest.test_05(null), pytest.test_05(undefined));

	console.log(pytest.test_06([1, 2, 3]).valueOf());

	console.log(pytest.test_07({'aaa': 111, 'bbb': 222}).valueOf());

	console.log(pytest.test_08({
		'aaa': 111, 222: 'bbb', 'ccc': true, 'ddd': 33.3, 
		'eee': function (x, y) { return x + y; }, 
		'fff': [444, '555', 666], 
		'ggg': {'hhh': 777, 888: 'iii', 'jjj': function (x, y) { return x * y; }}, 
		// 'kkk': this
	}).valueOf());

	console.log(pytest.test_09(function (caller, x, y, z) {
		console.log(caller, ' -> js_func_01');
		console.log('x: ', x.valueOf());
		console.log('y: ', y.valueOf());
		return z;
	})('js_test', [222, '333', 444], {'aaa': 555, 666: 'bbb', 'ccc': 777, 'ddd': 88.8})('js_test', 8, 9));

	try {
		pytest.test_10(function () { throw('js_test_exception'); } );
	} catch (e) {
		console.log('js_test -> ', e, typeof(e));
	}

	test_11 = pytest.Test_11();
	console.log(test_11.get_aaa());
	test_11.aaa = 22;
	console.log(test_11.get_aaa());
	test_11.bbb = 33;
	console.log(test_11.get_bbb());

	console.log(pytest.test_12[0], pytest.test_12[1], pytest.test_12[2]);
	pytest.test_12[0] = 4, pytest.test_12[1] = 5, pytest.test_12[2] = 6;
	console.log(pytest.test_12[0], pytest.test_12[1], pytest.test_12[2]);
	console.log(pytest.test_12[3]);
	pytest.test_12.append(7);
	console.log(pytest.test_12[3]);

	console.log(pytest.test_13['aaa'], pytest.test_13.bbb);
	pytest.test_13['aaa'] = 333, pytest.test_13.bbb = 444;
	console.log(pytest.test_13['aaa'], pytest.test_13.bbb);
	console.log(pytest.test_13['ccc']);
	pytest.test_13['ccc'] = 555;
	console.log(pytest.test_13['ccc']);
	console.log(pytest.test_13.ddd);
	pytest.test_13.ddd = 666;
	console.log(pytest.test_13.ddd);

	py_test_14 = pytest.test_14;
	js_test_14 = pytest.test_14.valueOf();
	console.log('py_test_14: ', typeof(py_test_14), 'js_test_14: ', typeof(js_test_14));
	console.log(py_test_14(8).toString(), js_test_14(8).toString());

	console.log(pytest.test_15(function () {
		return this.aaa;
	}.bind(pytest.test_15.data)));

	console.log('test_16 begin');
	pytest.test_16.async = true;
	console.log(pytest.test_16());
	console.log(pytest.test_16(false));
	pytest.test_16.async_cb = function (error, result) {
		console.log('js async error: ', error ? error.valueOf() : error, typeof(error));
		console.log('js async result: ', result);
	};
	console.log(pytest.test_16());
	console.log(pytest.test_16(false));
	console.log('test_16 end');

	console.log(pytest.test_17().valueOf());
}

for (var i = 0; i < 1; i++)
	test();
