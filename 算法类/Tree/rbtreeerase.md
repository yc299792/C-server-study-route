### 红黑树删除操作

红黑树删除有三种情况：

1. 删除节点没有子节点，也就是说该节点没有非空子节点，那么直接删除即可。
2. 删除的节点有一个子节点，那么用子节点顶替该节点即可。
3. 如果有两个非空子节点，那么就先找到该节点的后继节点，然后用后继节点顶替当前节点就可以了，然后删除后继节点，然后判断后继节点的子节点情况，后继节点只有可能会有一个子节点，若都为空，按1处理，有一个节点就按照2处理。

这里先规定三个变量分别是z待删除节点，y真正删除的节点，x轴心节点。

z:就是我们想要删除的那个节点，但是在实际操作中只是要去覆盖它的值而已，这个节点并没有被删除；

y：y就是我们要真的删除的节点，然后用这个节点的值去覆盖z的值，在1，2中，取y为z的子节点即可；

x：是我们操作的轴心节点，在后续进行平衡调整的时候就是以x为轴心的。

**下面来看看算法导论中的伪代码：**

```C
RB-DELETE(T, z)
    //这里就是先判断z是否有子节点
if left[z] = nil[T] or right[z] = nil[T] 
	//y实际删除的节点，对应到2
   then y ← z                               // 若“z的左孩子” 或 “z的右孩子”为空，则将“z”赋值给“y”；
   //这个就是1和3，去找到后继节点，后继节点就是要删除的节点了
   else y ← TREE-SUCCESSOR(z)               // 否则，将“z的后继节点”赋值给 “y”。
   //接下来就是要找轴心节点了，判断y也就是真正删除的节点左右孩子情况，只会有一个孩子的
if left[y] ≠ nil[T]
   then x ← left[y]                         // 若“y的左孩子” 不为空，则将“y的左孩子” 赋值给“x”；
   else x ← right[y]                        // 否则，“y的右孩子” 赋值给 “x”。
  //这一步就是要删除y了
p[x] ← p[y]                                 // 将“y的父节点” 设置为 “x的父节点”
if p[y] = nil[T]                               
   then root[T] ← x                         // 情况1：若“y的父节点” 为空，则设置“x” 为 “根节点”。
   else if y = left[p[y]]                    
           then left[p[y]] ← x             // 情况2：若“y是它父节点的左孩子”，则设置“x” 为 “y的父节//点的左孩子”
           else right[p[y]] ← x  // 情况3：若“y是它父节点的右孩子”，则设置“x” 为 “y的父节点的右孩子”
//y不等于z代表z是有两个非空的子节点的
if y ≠ z                                    
   then key[z] ← key[y]     // 若“y的值” 赋值给 “z”。注意：这里只拷贝y的值给z，而没有拷贝z的颜色！！
        
//如果真删除的是黑节点就破坏了黑平衡，所以需要调整
if color[y] = BLACK                            
   then RB-DELETE-FIXUP(T, x)                  // 若“y为黑节点”，则调用,以x为轴心调用调整
return y
```

然后调整就是以x为当前节点进行调整了：

插入操作中只有当前节点的父亲节点是红色的时候才需要调整，那么删除就是只有删除的这个节点是黑色的时候才需要调整，因为这样才会破坏黑平衡。

思想：就是不管x怎么样，先给加上黑色，然后去不断的移动这个黑色向根的方向。

```C
RB-DELETE-FIXUP(T, x)
  //如果现在x节点是黑色的也就是为了让黑平衡保持
while x ≠ root[T] and color[x] = BLACK  
    do if x = left[p[x]]     
    // 若 “x”是“它父节点的左孩子”，则设置 “w”为“x的叔叔”(即x为它父节点的右孩子)                                      
    		  then w ← right[p[x]]                                                           
               if color[w] = RED                                           // Case 1: x是“黑+黑”节点，x的兄弟节点是红色。(此时x的父节点和x的兄弟节点的子节点都是黑节点)。
                  then color[w] ← BLACK                        ▹  Case 1   //   (01) 将x的兄弟节点设为“黑色”。
                       color[p[x]] ← RED                       ▹  Case 1   //   (02) 将x的父节点设为“红色”。
                       LEFT-ROTATE(T, p[x])                    ▹  Case 1   //   (03) 对x的父节点进行左旋。
                       w ← right[p[x]]                         ▹  Case 1   //   (04) 左旋后，重新设置x的兄弟节点。
               if color[left[w]] = BLACK and color[right[w]] = BLACK       // Case 2: x是“黑+黑”节点，x的兄弟节点是黑色，x的兄弟节点的两个孩子都是黑色。
                  then color[w] ← RED                          ▹  Case 2   //   (01) 将x的兄弟节点设为“红色”。
                       x ←  p[x]                               ▹  Case 2   //   (02) 设置“x的父节点”为“新的x节点”。
                  else if color[right[w]] = BLACK                          // Case 3: x是“黑+黑”节点，x的兄弟节点是黑色；x的兄弟节点的左孩子是红色，右孩子是黑色的。
                          then color[left[w]] ← BLACK          ▹  Case 3   //   (01) 将x兄弟节点的左孩子设为“黑色”。
                               color[w] ← RED                  ▹  Case 3   //   (02) 将x兄弟节点设为“红色”。
                               RIGHT-ROTATE(T, w)              ▹  Case 3   //   (03) 对x的兄弟节点进行右旋。
                               w ← right[p[x]]                 ▹  Case 3   //   (04) 右旋后，重新设置x的兄弟节点。
                        color[w] ← color[p[x]]                 ▹  Case 4   // Case 4: x是“黑+黑”节点，x的兄弟节点是黑色；x的兄弟节点的右孩子是红色的。(01) 将x父节点颜色 赋值给 x的兄弟节点。
                        color[p[x]] ← BLACK                    ▹  Case 4   //   (02) 将x父节点设为“黑色”。
                        color[right[w]] ← BLACK                ▹  Case 4   //   (03) 将x兄弟节点的右子节设为“黑色”。
                        LEFT-ROTATE(T, p[x])                   ▹  Case 4   //   (04) 对x的父节点进行左旋。
                        x ← root[T]                            ▹  Case 4   //   (05) 设置“x”为“根节点”。
       else (same as then clause with "right" and "left" exchanged)        // 若 “x”是“它父节点的右孩子”，将上面的操作中“right”和“left”交换位置，然后依次执行。
color[x] ← BLACK
```

