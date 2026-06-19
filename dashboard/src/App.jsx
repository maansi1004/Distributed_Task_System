import { useEffect, useState } from "react";
import "./App.css";

function App() {
  const [metrics, setMetrics] = useState(null);
  const [tasks, setTasks] = useState([]);

  const fetchData = () => {
    fetch("http://localhost:8080/metrics")
      .then((res) => res.json())
      .then((data) => setMetrics(data));

    fetch("http://localhost:8080/tasks")
      .then((res) => res.json())
      .then((data) => setTasks(data));
  };

  useEffect(() => {
    fetchData();

    const interval = setInterval(
      fetchData,
      2000
    );

    return () => clearInterval(interval);
  }, []);

  if (!metrics) {
    return <h1>Loading...</h1>;
  }

  return (
    <div className="container">
      <h1 className="title">
        Task Queue Dashboard
      </h1>

      <div className="metrics-grid">
        <div className="card">
          <h3>Queue Depth</h3>
          <h2>{metrics.queue_depth}</h2>
        </div>

        <div className="card">
          <h3>Processing</h3>
          <h2>{metrics.processing_depth}</h2>
        </div>

        <div className="card">
          <h3>Success</h3>
          <h2>{metrics.success}</h2>
        </div>

        <div className="card">
          <h3>Failed</h3>
          <h2>{metrics.failed}</h2>
        </div>

        <div className="card">
          <h3>DLQ</h3>
          <h2>{metrics.dlq_depth}</h2>
        </div>
      </div>

      <div className="tasks-section">
        <h2>Recent Tasks</h2>

        <table>
          <thead>
            <tr>
              <th>ID</th>
              <th>Description</th>
              <th>Status</th>
              <th>Priority</th>
            </tr>
          </thead>

          <tbody>
            {tasks.map((task) => (
              <tr key={task.id}>
                <td>{task.id}</td>
                <td>{task.description}</td>
                <td>{task.status}</td>
                <td>{task.priority}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

export default App;