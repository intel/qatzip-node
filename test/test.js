/*
* Copyright (C) <2023-2023> Intel Corporation
* SPDX-License-Identifier: MIT
*/

"use strict";

const fs = require('fs');
const assert = require('assert');
const qzip = require('../lib/qzip.js');

const inputData = "Hello, World!";
const valid_level = 4; // Valid level within the range of 1-12

const src_buf = Buffer.from(inputData);
const option = { level: valid_level };

const compressed_buf1 = qzip.deflateSync(src_buf, option);
const decompressed_buf1 = qzip.inflateSync(compressed_buf1);
assert.strictEqual(Buffer.compare(src_buf, decompressed_buf1), 0, 'src_buf and decompressed_buf1 are equal');
console.log('deflateSync and inflateSync with valid parameters passed successfully!');


qzip.deflate(src_buf, option, (err, compressed_buf) => {
    if (err) {
        console.error(err);
        return;
    }

    qzip.inflate(compressed_buf, option, (err, result) => {
        if (err) {
        console.error(err);
        return;
        }
        const decompressed_buf = result;
        assert.strictEqual(Buffer.compare(src_buf, decompressed_buf), 0, 'src_buf and decompressed_buf are equal');
        console.log('deflate and inflate with valid parameters passed successfully!');
        console.log('All test passed successfully!');
    });
});

