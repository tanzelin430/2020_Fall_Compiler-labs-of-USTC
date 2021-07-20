# 简介

本实验是中国科学技术大学李诚老师2020秋编译原理的实验，我们实现了面向cminus-f的编译器，包括词法分析，语法分析，中间代码生成，编译器优化等方面，本实验获得了满分，代码部分详见[src](./src)目录

本实验由中国科学技术大学计算机系本科生：[谭泽霖](https://github.com/tanzelin430) [吕瑞](https://github.com/karin0018 ) 艾语晨共同完成

# 实验说明

请fork此`repo`到自己的仓库下，随后在自己的仓库中完成实验，请确保自己的`repo`为`Private`。

### 目前已布置的实验:
*   [lab1](./Documentations/lab1/README.md)
    *   DDL : 2020/10/13, 23:59:59
*   [lab2](./Documentations/lab2/README.md)
    *   DDL : 2020/10/30, 23:59:59
*   [lab3](./Documentations/lab3/README.md)
    *   DDL : 2020/11/16, 23:59:59
*   [lab4](./Documentations/lab4/README.md)
    *   DDL : 2020/12/07, 23:59:59
*   [lab5](./Documentations/lab5/README.md)
    *   DDL-Phase1 :2020/12/21 23:59:59 (北京标准时间，UTC+8)  
    *   DDL-Phase2 :2021/01/04 23:59:59 (北京标准时间，UTC+8) 
### FAQ: How to merge upstream remote branches
In brief, you need another alias for upstream repository (we assume you are now in your local copy of forked repository on Gitlab):
```
(shell) $ git remote add upstream http://222.195.68.197/staff/2020fall-compiler_cminus.git
```
Then try to merge remote commits to your local repository:
```
(shell) $ git pull upstream master
```
Then synchronize changes to your forked remote repository:
```
(shell) $ git push origin master
```
