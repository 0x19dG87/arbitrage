#  DEXes Arbitrage

This repository contains an arbitrage code designed for decentralized exchanges (DEXes) including Uniswap V3, Uniswap V2, and Sushiswap. The bot leverages The Graph protocol's subgraphs to identify and exploit price differences across these platforms.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Supported DEXes](#supported-dexes)
- [License](#license)

## Introduction

Arbitrage trading involves profiting from price discrepancies of the same asset across different markets. This code automates the arbitrage process for DEXes, ensuring efficient and timely trades using The Graph protocol's subgraphs + c++ library to find the shortest path.

## Features

- **Multi-DEX Support**: Works with Uniswap V3, Uniswap V2, and Sushiswap.
- **Subgraph Integration**: Uses The Graph protocol's subgraphs for real-time monitoring and data analysis.
- **Automated Trading**: !Not included! Only displays trade pairs when profitable opportunities are detected.

## Benefits of Using The Graph Protocol's Subgraphs

- **Real-Time Data**: Accesses up-to-date blockchain data without the need for complex indexing.
- **Efficiency**: Reduces latency in data retrieval, enhancing the bot's responsiveness.
- **Scalability**: Easily scales with the addition of new DEXes or trading pairs.
- **Simplicity**: Simplifies the integration process with a unified query interface.

## Installation

To set up the arbitrage, follow these steps:

1. **Clone the repository:**
   ```bash
   git clone https://github.com/0x19dG87/arbitrage.git
   cd arbitrage
   ```

2. **Install dependencies:**
   Ensure you have Node.js and npm installed. Then, install the required packages:
   ```bash
   npm install
   ```

3. **Set up environment variables:**
   Create a `.env` file in the project root with your Graph API keys (see [Configuration](#configuration) for details).

## Usage

To start the bot, run:

```bash
node index.js
```

The code will begin scanning for arbitrage opportunities and display trade pairs.

## Configuration

The code requires specific configurations set in a `.env` file in the project root. Below is a sample `.env` file:

```
API_KEY=your_graph_protocol_api_key
```

Replace the placeholder values with your actual settings.

## Supported DEXes

The code currently supports the following DEXes:

- Uniswap V3
- Uniswap V2
- Sushiswap

Support for additional DEXes can be added by extending the DEX integration modules.

## License

This project is licensed under the MIT License.