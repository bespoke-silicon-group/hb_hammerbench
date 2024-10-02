class Tag(object):
    # shift/mask for x
    X_SHIFT = 18
    X_MASK  = (1<<6)-1
    # shift/mask for y
    Y_SHIFT = 24
    Y_MASK  = (1<<6)-1
    # shift/mask for type
    TYPE_SHIFT = 30
    TYPE_MASK  = (1<<2)-1
    
    def __init__(self, word):
        self.word = word

    @property
    def x(self):
        return (self.word >> self.X_SHIFT) & self.X_MASK

    @property
    def y(self):
        return (self.word >> self.Y_SHIFT) & self.Y_MASK

    @property
    def type(self):
        typeint =  (self.word >> self.TYPE_SHIFT) & self.TYPE_MASK
        typestr = [
            'start'
            ,'end'
            ,'kernel_start'
            ,'kernel_end'
        ][typeint]
        return typestr
