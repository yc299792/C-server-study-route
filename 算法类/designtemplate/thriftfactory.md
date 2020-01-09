### thrift中工厂模式的使用-抽象工厂

我们以一个使用开始，在thrift中server模块有这么一个类TServer，就是服务器端的主类，这个类中有这么一行：

```  std::shared_ptr<TProtocolFactory> inputProtocolFactory_;//输入协议工厂指针```

可以看到，直接指明用的就是工厂模式，那么这个是怎么设计的：

TProtocolFactory这个类是这样的：

```C++
class TProtocolFactory {//这是协议的抽象工厂
public:
  TProtocolFactory() = default;

  virtual ~TProtocolFactory();

  virtual std::shared_ptr<TProtocol> getProtocol(std::shared_ptr<TTransport> trans) = 0;
  virtual std::shared_ptr<TProtocol> getProtocol(std::shared_ptr<TTransport> inTrans,
               std::shared_ptr<TTransport> outTrans) {
    (void)outTrans;
    return getProtocol(inTrans);
  }
};
```

这个类是一个抽象类，所以它就是协议工厂的接口类，下面肯定会有很多具体的协议工厂继承它，这就是C++利用多态实现接口的惯用套路。

我这里举一个具体的协议json吧：

```C++
class TJSONProtocolFactory : public TProtocolFactory {
public:
  TJSONProtocolFactory() = default;

  ~TJSONProtocolFactory() override = default;

  std::shared_ptr<TProtocol> getProtocol(std::shared_ptr<TTransport> trans) override {
    return std::shared_ptr<TProtocol>(new TJSONProtocol(trans));
  }
};
```

这就是源码哦，是不是就是继承了TProtocolFactory，所以说这个工厂就是用来创建json协议的工厂，肯定还有其他的，比如debug什么的，只要思路清楚了，分析代码就是根据自己的思路去验证。

然后接下来的想法就是肯定会有TJSONProtocolFactory工厂依赖的某一个类，好了我们看一下TProtocol，原来每一个具体的协议所返回的智能指针类型是TProtocol，所以这个类我么再去看看是怎么弄的，先想一下，返回的是TJSONProtocol，那么肯定是多态的用法，所以TProtocol肯定是抽象类，或者父类，反正会有继承：验证一下

```C++
class TProtocol {
    ...
    virtual uint32_t writeMessageBegin_virt(const std::string& name,
                                          const TMessageType messageType,
                                          const int32_t seqid) = 0;
    ...
};
```

果然没错吧，这个类就是一个抽象类，所以说后面具体的协议类肯定继承并重写了这些函数，只不过在thrift中不是直接继承的罢了。

```C++
class TProtocolDefaults : public TProtocol {
    ...
public:
      uint32_t writeMessageBegin(const std::string& name,
                             const TMessageType messageType,
                             const int32_t seqid) {
    (void)name;
    (void)messageType;
    (void)seqid;
    throw TProtocolException(TProtocolException::NOT_IMPLEMENTED,
                             "this protocol does not support writing (yet).");
  }
    ...
};

```



```C++
template <class Protocol_, class Super_ = TProtocolDefaults>
class TVirtualProtocol : public Super_ {
...
  uint32_t writeMessageBegin_virt(const std::string& name,
                                          const TMessageType messageType,
                                          const int32_t seqid) override {//重写了爷爷类中的虚函数
    return static_cast<Protocol_*>(this)->writeMessageBegin(name, messageType, seqid);//返回调用子类的方法
  }
  ...
};
```

他这里多封装了一下，增加了一个TProtocolDefaults，这个类里面主要是为了对协议的功能进行异常处理，稍等一下再看。

然后具体的json是这样写的：

```C++
class TJSONProtocol : public TVirtualProtocol<TJSONProtocol> {
public:
  TJSONProtocol(std::shared_ptr<TTransport> ptrans);

  ~TJSONProtocol() override;

private:
  void pushContext(std::shared_ptr<TJSONContext> c);

  void popContext();

  uint32_t writeJSONEscapeChar(uint8_t ch);//
    ...
        public:
  /**
   * Writing functions.
   */

  uint32_t writeMessageBegin(const std::string& name,
                             const TMessageType messageType,
                             const int32_t seqid);
    ....
};
```

是不是就有点通透了，还是不懂，再看看，具体的协议需要继承VirtualProtocol<TJSONProtocol>，类模板，然后类模板中干的事情就是，用父类指针去调用传入协议的功能方法，如果具体协议实现了这样的方法就会调用成功，如果在调用的时候子类没有覆盖那个方法，就会调用TProtocolDefaults，而这个类中的功能方法就是抛出没有实现的异常，这里就是对具体协议和抽象协议接口之前加了一层封装，实现了对功能的调用异常情况处理。

所以这里看一下就是多态的反复应用在server端调用接口类是一个TProtocolFactory(抽象工厂)<-------TJSONProtocolFactory（具体协议工厂）------------>TProtocol(前两个都依赖于抽象协议接口)<--------TVirtualProtocol<-------TJSONProtocol

大体就是这样是实现的，我这里就不画图了，所谓的依赖倒转就是，把逐级向下的箭头依赖方向倒转过来，这样就可以基于中间层进行扩展，从而实现了解耦，通过依赖倒转实现了开放封闭：** **对外可以增加模块或者可以扩展，所以是开放的，对内，在扩展的时候不需要修改内部的代码，所以是封闭的** **
