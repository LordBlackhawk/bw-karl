#ifndef SINGLETON_h
#define SINGLETON_h

template <class T>
  class Singleton
{
  public:
    static T& instance()
    {
      static T t;
      return t;
    }

  protected:
    Singleton()
    { }
};

#endif
