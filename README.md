#emWin SWIPELIST绕过删除Item后Attach窗口不自动对齐Bug

# 删除Item后附着在Item上的窗口不对齐Bug的现象

使用SWIPELIST的时候有时会附着在Item上面一些窗口用于显示某些信息或者某些选项，按钮等。
同样某些情况下我们需要删除Item但Item被删除后会自动对齐，而附着在其上的窗口却不会移动这应该是emWin的一个BUG
# 绕过此Bug的思路
删除Item后把SWIPELIST窗口内的可见的Item的附着窗口全部先Detach再Attach，且把附着窗口未显示的出来的部分`WM_ShowWin`使其显示。
## 所需注意此控件的一些特性（或坑）
1.在添加Item的时候SepSize在SeparatorItem之前的一个Item会出现异常，需要在添加完Item后再重新设置其SpearatorSize。
2.删除Item后在Detach 附着窗口再Attach上时虽然附着窗口已跟随到了Item的位置，但之前未显示的附着窗口 仍然不会显示，这时就需要`WM_ShowWin`来把未显示的显示出来。
3.显示未显示的 附着窗口时，**不能把所有的附着窗口的未显示窗口都显示出来。**否则会出现显示异常。
4当最后一个Item和其SepSize都==完整显示时==，如果其后还有Item那么这个Item的附着窗口也需要`WM_ShowWin`出来，否则滑动SWIPELIST表时接下来的附着窗口不会显示
## 具体思路
1.首先得计算出SWIPELIST窗口中可以完整显示的Item个数。
2.计算出SWIPELIST窗口中是否有==未完全显示的部分已及这部分的Size。==
3.根据当前的ScrollPos计算出SWIPELIST窗口顶部Item是否移出一部分已及==移出量==
4.删除Item后根据`1,2,3`得出最后一个Item是否完全显示。
5.根据`1,4`算出需要处理的Item个数。
如此便可以绕过这个BUG正常显示和操作。
# 实现细节和代码
## 管理数据
在绕过这个BUG的时候我也做了一个其它的实现：点击选定Item的时候Item背景色改变指示其选定状态，更改上一个Item的选定状态。
而且由于：
	1.我们需要对AttachWin的频繁操作，AttachWin和Item又是对应起来的。
	2.Item是可以增删，不定长的。
	3.在添加Item的时候不可随意指定其添加位置（如把Item插入到某两个Item之间）而很多时候我时候我们都需要在Item中添加一个或多个SeparatorItem来分组不同类别的Item。我们必要要按照顺序一组一组的添加Item到SWIPELIST中去。
因此这里我使用了链表，有列表和列表项的思想在里面。SeparatorItem表包含SeparatorItem。在SeparatorItem项中又有一个普通Item的列表，这些表项都存储着其Item信息，如是否有`焦点`，其附着窗口`句柄`，Item`名称`，Item所开辟空间的`句柄`等。
以下部分代码展示的是创建一个窗口来放置SWIPELSIT并创建SWIPELSIT和一个按钮来模拟应用中删除Item，和模拟应用中添加Item和SeparatorItem。以及把链表中的数据遍历后添加Item到SWIPELSIT中去且把每一个Item数据项的的指针传递给Item做其UserData