/*
* Copyright (C) <2023-2023> Intel Corporation
* SPDX-License-Identifier: MIT
*/

"use strict";

const zlib = require('zlib');
let qzipAddon;
let useZlibFallback = false;

try {
  qzipAddon = require('bindings')('qzip.node');
} catch (error) {
  console.error('Failed to enable QAT environment, falling back to zlib: ');
  useZlibFallback = true;
}


function getOptions(opts) {
  const defaultOpts = {
      level: 4,
  };

  if (opts) {
      if (opts.level) {
          if (typeof opts.level !== 'number' || opts.level < 1 || opts.level > 12) {
              throw new Error('Invalid level: must be a number between 1 and 12');
          }
      }
  }

  return Object.assign(defaultOpts, opts);
}


function deflateSync(src_buf, opts = {}) {
  const option = getOptions(opts);
  return qzipAddon.deflateSync(src_buf, option); 
}

function inflateSync(src_buf, opts = {}) {
  const option = getOptions(opts);
  return qzipAddon.inflateSync(src_buf, option);
}

function deflateAsync(src_buf, options = {}, callback) {
  if (typeof src_buf !== 'object' || !Buffer.isBuffer(src_buf)) {
    throw new TypeError('src_buf must be a Buffer object');
  }
  if (callback) {
    const option = getOptions(options);
    qzipAddon.deflateAsync(src_buf, option, callback);
  } else {
    return new Promise((resolve, reject) => {
      const option = getOptions(options);
      qzipAddon.deflateAsync(src_buf, option, (err, compressedData) => {
        if (err) {
          reject(err);
        } else {
          resolve(compressedData);
        }
      });
    });
  }
}

function inflateAsync(src_buf, options = {}, callback) {
if (callback) {
  const option = getOptions(options);
  qzipAddon.inflateAsync(src_buf, option, callback);
} else {
  return new Promise((resolve, reject) => {
    const option = getOptions(options);
    qzipAddon.inflateAsync(src_buf, option, (err, decompressedData) => {
      if (err) {
        reject(err);
      } else {
        resolve(decompressedData);
      }
    });
  });
}
}



module.exports = useZlibFallback
  ? {
      deflateSync: zlib.deflateSync,
      inflateSync: zlib.inflateSync,
      deflate: zlib.deflate,
      inflate: zlib.inflate,
    }
  : {
      deflateSync,
      inflateSync,
      deflate: deflateAsync,
      inflate: inflateAsync,
    };