from m5.objects import Cache

# Base Cache
class L1Cache(Cache):
    # No default values here - will be set from options
    
    #Connect CPu to the cache
    def connectCPU(self, cpu):
        # need to define this in a base class!
        raise NotImplementedError
        
    # Connect the cache to the bus
    def connectBus(self, bus):
        self.mem_side = bus.cpu_side_ports

    def __init__(self, options=None): #Makes passing of parameters optional
        super(L1Cache, self).__init__()
        if not options:
            return
            
        # Common L1 cache parameters - set these once in base class
        self.tag_latency = 2
        self.data_latency = 2
        self.response_latency = 2
        self.mshrs = 4
        self.tgts_per_mshr = 20
        
        # Allow JSON config to override defaults
        if hasattr(options, 'l1_tag_latency'):
            self.tag_latency = options.l1_tag_latency
        if hasattr(options, 'l1_data_latency'):
            self.data_latency = options.l1_data_latency
        if hasattr(options, 'l1_response_latency'):
            self.response_latency = options.l1_response_latency
        if hasattr(options, 'l1_mshrs'):
            self.mshrs = options.l1_mshrs
        if hasattr(options, 'l1_tgts_per_mshr'):
            self.tgts_per_mshr = options.l1_tgts_per_mshr

class L1ICache(L1Cache):
    #Has to be defined here because cache port name is different
    def connectCPU(self, cpu):
        self.cpu_side = cpu.icache_port

    def __init__(self, options=None):
        super(L1ICache, self).__init__(options)
        if not options:
            return
            
        # Only set I-cache specific parameters
        if hasattr(options, 'l1i_size'):
            self.size = options.l1i_size
        if hasattr(options, 'l1i_assoc'):
            self.assoc = options.l1i_assoc
        
        # Override base class settings if specifically set
        if hasattr(options, 'l1i_tag_latency'):
            self.tag_latency = options.l1i_tag_latency
        if hasattr(options, 'l1i_data_latency'):
            self.data_latency = options.l1i_data_latency
        if hasattr(options, 'l1i_response_latency'):
            self.response_latency = options.l1i_response_latency
        if hasattr(options, 'l1i_mshrs'):
            self.mshrs = options.l1i_mshrs
        if hasattr(options, 'l1i_tgts_per_mshr'):
            self.tgts_per_mshr = options.l1i_tgts_per_mshr

class L1DCache(L1Cache):
    #Has to be defined here because cache port name is different
    def connectCPU(self, cpu):
        self.cpu_side = cpu.dcache_port

    def __init__(self, options=None):
        super(L1DCache, self).__init__(options)
        if not options:
            return
            
        # Only set D-cache specific parameters
        if hasattr(options, 'l1d_size'):
            self.size = options.l1d_size
        if hasattr(options, 'l1d_assoc'):
            self.assoc = options.l1d_assoc
            
        # Override base class settings if specifically set
        if hasattr(options, 'l1d_tag_latency'):
            self.tag_latency = options.l1d_tag_latency
        if hasattr(options, 'l1d_data_latency'):
            self.data_latency = options.l1d_data_latency
        if hasattr(options, 'l1d_response_latency'):
            self.response_latency = options.l1d_response_latency
        if hasattr(options, 'l1d_mshrs'):
            self.mshrs = options.l1d_mshrs
        if hasattr(options, 'l1d_tgts_per_mshr'):
            self.tgts_per_mshr = options.l1d_tgts_per_mshr

# Second Level Cache, connects between l2 Bus and memory bus
class L2Cache(Cache):

    # Default values for L2 cache
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 20
    tgts_per_mshr = 12
    
    #Connect CPU side bus to the cache
    def connectCPUSideBus(self, bus):
        self.cpu_side = bus.mem_side_ports
        
    #Connect cache to memory bus
    def connectMemSideBus(self, bus):
        self.mem_side = bus.cpu_side_ports

    def __init__(self, options=None):
        super(L2Cache, self).__init__()
        if not options:
            return
            
        # Set L2 cache parameters
        if hasattr(options, 'l2_size'):
            self.size = options.l2_size
        if hasattr(options, 'l2_assoc'):
            self.assoc = options.l2_assoc
        if hasattr(options, 'l2_tag_latency'):
            self.tag_latency = options.l2_tag_latency
        if hasattr(options, 'l2_data_latency'):
            self.data_latency = options.l2_data_latency
        if hasattr(options, 'l2_response_latency'):
            self.response_latency = options.l2_response_latency
        if hasattr(options, 'l2_mshrs'):
            self.mshrs = options.l2_mshrs
        if hasattr(options, 'l2_tgts_per_mshr'):
            self.tgts_per_mshr = options.l2_tgts_per_mshr