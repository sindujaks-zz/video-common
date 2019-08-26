# Copyright (c) 2019 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import socket
import logging as log
import time
import os


class Util:

    @staticmethod
    def check_port_availability(hostname, port):
        """Verifies port availability on hostname for accepting connection

        :param hostname: hostname of the machine
        :type hostname: str
        :param port: port
        :type port: str
        :return: portUp (whether port is up or not)
        :rtype: Boolean
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        log.debug("Attempting to connect to {}:{}".format(hostname, port))
        numRetries = 1000
        retryCount = 0
        portUp = False
        while(retryCount < numRetries):
            if(sock.connect_ex((hostname, int(port)))):
                log.debug("{} port is up on {}".format(port, hostname))
                portUp = True
                break
            retryCount += 1
            time.sleep(0.1)
        return portUp

    @staticmethod
    def get_topics_from_env(topic_type):
        """Returns a list of all topics the module needs to subscribe
        or publish

        :param topic_type: type of the topic(pub/sub)
        :type topic_type: str
        :return: list of topics in PubTopics
        :rtype: list
        """
        if topic_type == "pub":
            topicsList = os.environ["PubTopics"].split(",")
        elif topic_type == "sub":
            topicsList = os.environ["SubTopics"].split(",")

        return topicsList

    @staticmethod
    def get_messagebus_config(topic, topic_type,
                              zmq_clients, config_client, dev_mode):
        """Returns the config associated with the corresponding topic

        :param topic: name of the topic
        :type topic: str
        :param topic_type: type of the topic(pub/sub)
        :type topic_type: str
        :param zmq_clients: List of subscribers when used by publisher
                            or publisher string when used by subscriber
        :type zmq_clients: list(publisher) or string(subscriber)
        :param config_client: Used to get keys value from ETCD.
        :type config_client: Class Object
        :param dev_mode: check if dev mode or prod mode
        :type dev_mode: Boolean
        :raises ValueError: Raise value error if topic_type is not valid
        :raises ValueError: Raise value error if mode is not valid
        :return: config dict of corresponding topic
        :rtype: dict
        """
        app_name = os.environ["AppName"]
        topic = topic.strip()
        mode, address = os.environ[topic + "_cfg"].split(",")
        mode = mode.strip()
        address = address.strip()
        if mode == "zmq_tcp":
            host, port = address.split(":")
            config = {
                        "type": mode
            }
            host_port_details = {
                  "host":   host,
                  "port":   int(port)
            }
            topic_type = topic_type.lower()
            if topic_type == "pub":
                config["zmq_tcp_publish"] = host_port_details

                if not dev_mode:
                    allowed_clients = []
                    for subscriber in zmq_clients:
                        subscriber = subscriber.strip()
                        allowed_clients_keys = config_client.GetConfig(
                                        "/Publickeys/{0}".format(subscriber))
                        if allowed_clients_keys is not None:
                            allowed_clients.append(allowed_clients_keys)

                    config["allowed_clients"] = allowed_clients
                    config["zmq_tcp_publish"]["server_secret_key"] = \
                        config_client.GetConfig("/" + app_name +
                                                "/private_key")

            elif topic_type == "sub":
                config[topic] = host_port_details
                if not dev_mode:
                    zmq_clients = zmq_clients.strip()

                    config[topic]["server_public_key"] = \
                        config_client.GetConfig("/Publickeys/{0}".
                                                format(zmq_clients))

                    config[topic]["client_public_key"] = \
                        config_client.GetConfig("/Publickeys/" + app_name)

                    config[topic]["client_secret_key"] = \
                        config_client.GetConfig("/" + app_name +
                                                "/private_key")
            else:
                log.error("{} type is not valid".format(topic_type))
        elif mode == "zmq_ipc":
            config = {
                        "type":   mode,
                        "socket_dir":   address
                    }
        else:
            log.error("{} mode is not valid".format(mode))

        return config
