# EpiScriptorium

![Playground](https://img.shields.io/badge/Status-Experimental%20Playground-orange?style=for-the-badge)

> [!Note] 
> 本仓库是个人的一个实验场，几乎所有脚本均借助大语言模型快速实现，可能存在粗糙边界情况处理，建议在非关键环境中使用。

个人效率工具集 - 适用于Windows的自动化脚本集合

## 📖 简介

本仓库存储了 Epi 日常提升电脑使用效率的自动化脚本（ezScript），相关软件的配置文件（exConfig），使用软件的名单记录（esWarelog），个人实验性学习的测试脚本（etBatch） 等等。

## 🛠️ 功能清单

### 🔍 环境检测类
- `env-check.ps1`  

  Windows下经常需要增改环境变量，在软件管理时一不留神就容易漏删漏改，env-check是一个检查环境变量是否仍然合法的一个工具，其遍历所有的系统变量及用户变量，并检查其路径是否合法，统计并输出相应日志信息。

## 🚀 TODO

- [ ] 自动清理无效环境变量
- [ ] 文件分类整理工具
- [ ] 软件管理记录工具