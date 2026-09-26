using System;
using System.IO;
using System.Net.Sockets;
using System.Text;

namespace Cliente
{
    public class ManejarConexion : IDisposable
    {
        private TcpClient? _cliente;

		private StreamReader? _lector;

		private StreamWriter? _escritor;

		public bool Conectado => _cliente != null && _cliente.Connected;

		public void Conectar(string ip, int puerto)
		{
			_cliente = new TcpClient(ip, puerto);
			var stream = _cliente.GetStream();
			_lector = new StreamReader(stream, Encoding.UTF8);
			_escritor = new StreamWriter(stream, Encoding.UTF8) {AutoFlush = true};
        }

		public void EnviarMensaje(string json)
		{
			if (Conectado && _escritor != null)
			{
				_escritor.Write(json + "\n");
			}
		}

		public string? LeerMensaje()
		{
			if (_lector == null) return null;
			return _lector.ReadLine();
		}

		public void Dispose()
		{
			_lector?.Dispose();
			_escritor?.Dispose();
			_cliente?.Close();
		}
    }
}
