// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
example = require('../build/vs14_node/Debug/jsbind-example');

var i = example.hello(10, 'asdasfoo');

console.log('got', i, 'from c++');

var j = example.pipi.bar('asdas', 3.14);

console.log('pipi gave me', j);

console.log(asd);
