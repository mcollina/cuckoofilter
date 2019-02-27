'use strict'

const binding = require('node-gyp-build')(__dirname)

function CuckooFilter (totalSize, bitsPerItem = 12) {
  switch (bitsPerItem) {
    case 2:
    case 4:
    case 8:
    case 12:
    case 16:
    case 32:
      return new binding['CuckooFilter' + bitsPerItem](totalSize)
    default:
      throw new Error('The second argument must be: 2, 4, 8, 12, 16, or 32.')
  }
}

module.exports = CuckooFilter
